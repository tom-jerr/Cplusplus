#ifndef _CLCONDITION_H
#define _CLCONDITION_H
#include <condition_variable>
#include <mutex>
/**
 * @brief 条件变量类
 * @note 用于线程同步，对condition_variable的封装
 */
class Condition {
 public:
  explicit Condition() = default;
  ~Condition() = default;

  Condition(const Condition&) = delete;
  const Condition& operator=(const Condition&) = delete;

  void wait(std::unique_lock<std::mutex>& lock) { m_condition_.wait(lock); }
  void waitForSeconds(std::unique_lock<std::mutex>& lock, double seconds) {
    const int64_t kNanoSecondsPerSecond = 1000000000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    m_condition_.wait_for(lock, std::chrono::nanoseconds(nanoseconds));
  }

  void notify() { m_condition_.notify_one(); }
  void notifyAll() { m_condition_.notify_all(); }

 private:
  std::condition_variable m_condition_;
};

#endif /* _CLCONDITION_H */