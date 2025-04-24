/**
 * @file FileUring.cpp
 * @author lzy
 * @brief
 * @version 0.1
 * @date 2025-04-23
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "muduo/net/iouringfile/FileUring.h"
#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"
#include "muduo/net/RequestContext.h"
#include <algorithm>
#include <cassert>
#include <fcntl.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <memory>
namespace muduo {
namespace net {
FileUring::FileUring(const std::string &file_path, EventLoop *loop,
                     bool whether_direct)
    : file_path_(file_path), whether_direct_(whether_direct) {
  if (whether_direct_) {
    file_channel_ = new Channel(
        loop, ::open(file_path.c_str(), O_CREAT | O_RDWR | O_DIRECT | O_CLOEXEC,
                     0644));
  } else {
    file_channel_ = new Channel(
        loop, ::open(file_path.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0644));
  };
}

FileUring::~FileUring() {
  if (file_channel_) {
    // file_channel_->disableAll();
    // file_channel_->remove();
    delete file_channel_;
    file_channel_ = nullptr;
  }
}

std::future<bool> FileUring::asyncRead(void *buffer, size_t data_size,
                                       size_t offset,
                                       std::function<void()> callback) {
  if (file_channel_) {
    auto promise_ptr = std::make_shared<std::promise<bool>>();
    auto future = promise_ptr->get_future();
    RequestContext *context = new RequestContext(
        file_channel_, buffer, data_size, offset, request_id_,
        RequestType::Read, promise_ptr, callback);

    // request_map_.insert({request_id_, context});
    // LOG_INFO << "asyncRead context: " << context;
    file_channel_->setUringReadCallback([](struct io_uring_cqe *cqe) {
      LOG_INFO << "Read callback triggered";
      RequestContext *cqe_context = nullptr;
      // 1. handel cqe io finished
      if (cqe->res < 0) {
        LOG_ERROR << "I/O error: " << strerror(cqe->res)
                  << " (user_data=" << cqe->user_data << ")";
      } else {
        cqe_context = reinterpret_cast<RequestContext *>(cqe->user_data);
        if (cqe_context == nullptr) {
          LOG_ERROR << "user_data is null";
          return;
        }
        assert(cqe_context->getRequestType() == RequestType::Read);
        LOG_INFO << "Request Id: " << cqe_context->getReqId()
                 << ", Read from file, data size: " << cqe->res
                 << ", user_data: "
                 << reinterpret_cast<char *>(cqe_context->getBuffer());

        cqe_context->execUserCallback();
      }
      cqe_context->setComplete();
      // 处理完成后删除请求上下文
      delete cqe_context;
    });
    // file_channel_->setType(RequestType::Read);
    file_channel_->ownerLoop()->submitUringRequest(context);
    // wake up the event loop
    file_channel_->ownerLoop()->wakeup();
    request_id_++;
    return future;
  } else {
    LOG_ERROR << "File channel is null";
    return std::future<bool>();
  }
}

std::future<bool> FileUring::asyncWrite(void *str, size_t data_size,
                                        size_t offset,
                                        std::function<void()> callback) {
  if (file_channel_) {
    auto promise_ptr = std::make_shared<std::promise<bool>>();
    auto future = promise_ptr->get_future();
    RequestContext *context =
        new RequestContext(file_channel_, str, data_size, offset, request_id_,
                           RequestType::Write, promise_ptr, callback);

    // request_map_.insert({request_id_, context});
    // callback_map_.insert({request_id_, callback});
    file_channel_->setUringWriteCallback([](struct io_uring_cqe *cqe) {
      // LOG_INFO << "Write callback triggered";
      RequestContext *cqe_context = nullptr;
      // 1. handel cqe io finished
      if (cqe->res < 0) {
        LOG_ERROR << "I/O error: " << strerror(cqe->res)
                  << " (user_data=" << cqe->user_data << ")";
      } else {
        cqe_context = reinterpret_cast<RequestContext *>(cqe->user_data);

        if (cqe_context == nullptr) {
          LOG_ERROR << "user_data is null";
          return;
        }
        assert(cqe_context->getRequestType() == RequestType::Write);
        LOG_INFO << "Request id: " << cqe_context->getReqId()
                 << ", Write to file, data size: " << cqe->res;
        cqe_context->execUserCallback();
      }

      cqe_context->setComplete();
      // 处理完成后删除请求上下文
      delete cqe_context;
    });

    // file_channel_->setType(RequestType::Write);
    file_channel_->ownerLoop()->submitUringRequest(context);
    // wake up the event loop
    file_channel_->ownerLoop()->wakeup();
    request_id_++;
    return future;
  } else {
    LOG_ERROR << "File channel is null";
    return std::future<bool>();
  }
}
} // namespace net

} // namespace muduo