#include "../../include/logger/CLThread.hpp"

#include <cassert>

Thread::Thread(const ThreadFunc& func)
    : m_func_(func), m_started_(false), m_joined_(false) {}

Thread::~Thread() {
  if (m_started_ && !m_joined_) {
    m_thread_->detach();
  }
}

void Thread::start() {
  assert(!m_started_);
  assert(!m_thread_);
  m_thread_.reset(new std::thread(m_func_));
  m_started_ = true;
}

void Thread::join() {
  assert(m_started_);
  assert(!m_joined_);
  m_thread_->join();
}

void Thread::detach() {
  assert(m_started_);
  assert(!m_joined_);
  m_thread_->detach();
}