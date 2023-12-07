#ifndef CLOUTERSORT_H_
#define CLOUTERSORT_H_
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

#include "CLSafeQueue.h"
#ifdef BIGFILE
#define LIMITMEM 1024 * 1024 * 16  // 限制内存大小
#else
#define LIMITMEM (1000 * 8)  // 限制内存大小
#endif
namespace neo {
class OuterSort {
  using Task = std::function<void()>;

 private:
  /*
   线程池中结构
 */
  std::vector<std::thread> m_workers_;  // 线程池
  std::thread m_merge_worker_;          // 加入merge任务的线程
  SafeQueue<Task> m_tasks_;             // 任务队列
  SafeQueue<std::string> m_files_;      // 文件队列
  long long m_num_;                     // 数据的数量
  /*
    互斥锁，条件变量
  */
  std::mutex m_mutex_;                   // 线程池互斥锁
  std::mutex m_file_mutex_;              // 文件队列互斥锁
  std::condition_variable m_cond_;       // 线程池条件变量
  std::condition_variable m_file_cond_;  // 文件队列条件变量
  /*
    原子变量
  */
  std::atomic<bool> m_stop_;               // 是否结束线程池中线程
  std::vector<std::string> m_need_files_;  // 需要排序的总文件

 private:
  // 线程池每次执行的函数
  void process_task();
  // merge线程每次执行的函数
  void merge_task();
  template <typename F, typename... Args>
  auto enqueue_task(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type>;
  void enqueue_file(std::string file);
  void sort(const std::string &path);
  void merge(const std::string &path1, const std::string &path2);
  void separate_sort(const std::string &path);

 public:
  OuterSort(size_t thread_num, std::vector<std::string> need_files);
  ~OuterSort();
  std::string run();
};
}  // namespace neo

#endif /* CLOUTERSORT_H_ */