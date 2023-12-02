#ifndef SAFEm_queue_H_
#define SAFEm_queue_H_
#include <condition_variable>
#include <mutex>
#include <queue>

namespace neo {

template <typename T>
class SafeQueue {
 private:
  std::queue<T> m_queue_;  // use STL queue to store data
  std::mutex m_mutex_;     // the mutex to synchronise on
  std::condition_variable m_full_;
  std::condition_variable m_empty_;

 public:
  SafeQueue() = default;
  ~SafeQueue() = default;
  void enqueue(T t);   // enqueue an object
  void dequeue(T& t);  // dequeue an object
  bool empty();        // check if queue is empty
  int size();          // return the size of queue

  void wakeup();
};

template <typename T>
bool SafeQueue<T>::empty() {
  std::lock_guard<std::mutex> lock(m_mutex_);
  return m_queue_.empty();
}

template <typename T>
int SafeQueue<T>::size() {
  std::lock_guard<std::mutex> lock(m_mutex_);
  return m_queue_.size();
}

template <typename T>
void SafeQueue<T>::enqueue(T t) {
  std::unique_lock<std::mutex> lock(m_mutex_);
  // 过于庞大的任务队列，阻塞生产者
  while (m_queue_.size() > 1000) {
    m_full_.wait(lock);
  }
  m_queue_.emplace(t);
  m_empty_.notify_one();
}

template <typename T>
void SafeQueue<T>::dequeue(T& t) {
  std::unique_lock<std::mutex> lock(m_mutex_);
  while (m_queue_.empty()) {
    m_empty_.wait(lock);
  }
  t = std::move(m_queue_.front());
  m_queue_.pop();
}

template <typename T>
void SafeQueue<T>::wakeup() {
  m_empty_.notify_all();
}
}  // namespace neo

#endif