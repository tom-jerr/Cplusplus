#ifndef _CLTHREAD_H
#define _CLTHREAD_H

#include <functional>
#include <memory>
#include <thread>

/**
 * @brief Thread class，对std::thread进行封装；可以支持多种Callable对象
 */
class Thread {
 public:
  using ThreadFunc = std::function<void()>;
  explicit Thread(const ThreadFunc& func);
  Thread(const Thread&) = delete;
  Thread& operator=(const Thread&) = delete;
  ~Thread();

  void start();
  void join();
  void detach();

  bool started() const { return m_started_; }
  bool joined() const { return m_joined_; }

 private:
  std::unique_ptr<std::thread> m_thread_;
  ThreadFunc m_func_;
  bool m_started_;
  bool m_joined_;
};

#endif /* _CLTHREAD_H */