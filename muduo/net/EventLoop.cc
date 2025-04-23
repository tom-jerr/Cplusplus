// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/EventLoop.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/Channel.h"
#include "muduo/net/Poller.h"
#include "muduo/net/RequestContext.h"
#include "muduo/net/SocketsOps.h"
#include "muduo/net/TimerQueue.h"
#include <algorithm>
#include <liburing.h> // Provides the definition of struct io_uring
#include <liburing/io_uring.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;
#define ENTRIES 1024
namespace {
__thread EventLoop *t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
} // namespace

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false), eventHandling_(false),
      callingPendingFunctors_(false), iteration_(0),
      threadId_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this)),
      timerQueue_(std::make_unique<TimerQueue>(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
      currentActiveChannel_(NULL), event_fd_(createEventfd()),
      event_channel_(std::make_unique<Channel>(this, event_fd_)) {
  struct io_uring ring;
  struct io_uring_params params;
  memset(&params, 0, sizeof(params));
  params.flags = 0;
  params.cq_entries = ENTRIES;
  params.sq_entries = ENTRIES;
  // params.flags = 0; // 可选：使用内核轮询提升性能
  auto ret = io_uring_queue_init_params(ENTRIES, &ring, &params);

  if (ret < 0) {
    LOG_SYSERR << "io_uring 初始化失败: " << strerror(-ret);
    abort();
  }

  io_uring_ = std::make_shared<struct io_uring>(ring);
  ret = io_uring_register_eventfd(&ring, event_fd_);
  if (ret < 0) {
    LOG_SYSERR << "注册 eventfd 失败: " << strerror(-ret);
    close(event_fd_);
    io_uring_queue_exit(&ring);
    return;
  }
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  // we are always reading the wakeupfd
  wakeupChannel_->enableReading();

  event_channel_->setReadCallback(
      std::bind(&EventLoop::handleUringReqComplete, this));
  // we are always reading the eventfd
  event_channel_->enableReading();
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << CurrentThread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

/**
 * @brief for iouring function
 *
 */
void EventLoop::submitUringRequest(RequestContext *context) {
  // 1. get sqe
  struct io_uring_sqe *sqe = io_uring_get_sqe(io_uring_.get());
  if (!sqe) {
    LOG_ERROR << "无法获取 SQE";
    return;
  }
  // 2. prepare request
  if (context->getRequestType() == RequestType::Read) {
    io_uring_prep_read(sqe, context->getChannel()->fd(), context->getBuffer(),
                       static_cast<unsigned int>(context->getDataSize()),
                       static_cast<__u64>(context->getOffset()));
  } else if (context->getRequestType() == RequestType::Write) {
    io_uring_prep_write(sqe, context->getChannel()->fd(), context->getBuffer(),
                        static_cast<unsigned int>(context->getDataSize()),
                        static_cast<unsigned int>(context->getOffset()));
  } else {
    LOG_ERROR << "未知请求类型: "
              << static_cast<int>(context->getRequestType());
    return;
  }
  // 3. set user data
  sqe->user_data = reinterpret_cast<__u64>(context);
  // 4. submit request
  int ret = io_uring_submit(io_uring_.get());
  if (ret < 0) {
    LOG_ERROR << "提交 SQEs 失败: " << strerror(-ret);
    return;
  }
}

void EventLoop::handleUringReqComplete() {
  // 1. read from eventfd
  uint64_t event_count;
  ssize_t s = read(event_fd_, &event_count, sizeof(event_count));
  if (s != sizeof(event_count)) {
    LOG_ERROR << "读取 eventfd 失败";
    return;
  }
  // 2. handle all ready CQE
  struct io_uring_cqe *cqe;
  unsigned head;
  unsigned count = 0;
  io_uring_for_each_cqe(io_uring_.get(), head, cqe) {
    if (cqe->res < 0) {
      LOG_ERROR << "I/O 错误: " << strerror(cqe->res)
                << " (user_data=" << cqe->user_data << ")";
    } else {
      LOG_INFO << "CQE user data: " << cqe->user_data;
      RequestContext *context =
          reinterpret_cast<RequestContext *>(cqe->user_data);
      if (context == nullptr) {
        LOG_ERROR << "user_data is null";
        continue;
      }
      if (context->getRequestType() == RequestType::Read) {
        context->getChannel()->setCQE(cqe);
        context->getChannel()->callUringReadCallback();

      } else if (context->getRequestType() == RequestType::Write) {
        context->getChannel()->setCQE(cqe);
        context->getChannel()->callUringWriteCallback();
      } else {
        LOG_ERROR << "未知请求类型: "
                  << static_cast<int>(context->getRequestType());
      }
    }
    count++;
  }
  // update cqe
  io_uring_cq_advance(io_uring_.get(), count);
  if (count > 0) {
    LOG_INFO << "处理完成的请求数量: " << count;
  } else {
    LOG_INFO << "没有处理完成的请求";
  }
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false; // FIXME: what if someone calls quit() before loop() ?
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    ++iteration_;
    if (Logger::logLevel() <= Logger::TRACE) {
      printActiveChannels();
    }
    // TODO sort channel by priority
    eventHandling_ = true;
    for (Channel *channel : activeChannels_) {
      currentActiveChannel_ = channel;
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }
    currentActiveChannel_ = NULL;
    eventHandling_ = false;
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor cb) {
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

size_t EventLoop::queueSize() const {
  MutexLockGuard lock(mutex_);
  return pendingFunctors_.size();
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) { return timerQueue_->cancel(timerId); }

void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_) {
    assert(currentActiveChannel_ == channel ||
           std::find(activeChannels_.begin(), activeChannels_.end(), channel) ==
               activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (const Functor &functor : functors) {
    functor();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const {
  for (const Channel *channel : activeChannels_) {
    LOG_TRACE << "{" << channel->reventsToString() << "} ";
  }
}
