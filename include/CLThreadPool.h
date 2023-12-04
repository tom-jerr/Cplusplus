#ifndef THREADPOOL_H_
#define THREADPOOL_H_
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
// #include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

// #include "ConcurrentQueue.h"
#include "CLSafeQueue.h"
namespace neo {

class ThreadPool {
  using Task = std::function<void()>;

 private:
  std::vector<std::thread> m_workers_;  // 线程池
  SafeQueue<Task> m_tasks_;             // 任务队列
  SafeQueue<std::string> m_files_;      // 文件队列
  std::mutex m_mutex_;                  // 线程池互斥锁
  std::condition_variable m_cond_;      // 线程池条件变量
  std::atomic<bool> m_stop_;            // 是否结束线程池中线程
  std::atomic<int> m_threadnum_;        // 线程池中线程数量
 private:
  // 线程池每次执行的函数
  void process_task();
  void merge_task(const std::string &file3);
  void write_data(FILE *f, int a[], int n);
  void read_data(FILE *f, int a[], int n);

 public:
  ThreadPool(size_t thread_num = 4);
  ~ThreadPool();
  template <typename F, typename... Args>
  auto enqueue_task(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type>;
  void enqueue_file(std::string file);
  void sort(const std::string &path, const std::string &tmpfile);
  void merge(const std::string &path1, const std::string &path2,
             const std::string &path3);
};
// // 向文本文件中写入数据
// inline void ThreadPool::write_data(FILE *f, uint64_t a[], int n) {
//   for (int i = 0; i < n; ++i) fprintf(f, "%d ", a[i]);
// }
// // 从文本文件中读取数据
// inline void ThreadPool::read_data(FILE *f, int a[], int n) {
//   for (int i = 0; i < n; ++i) fscanf(f, "%d", &a[i]);
// }

// // 单个文件排序
// inline void ThreadPool::sort(const std::string &path,
//                              const std::string &tmpfile) {
//   FILE *fp = fopen(path.c_str(), "r+");
//   FILE *tmp = fopen(tmpfile.c_str(), "wb+");
//   fseek(fp, 0, SEEK_END);
//   int len = ftell(fp) / sizeof(int);
//   fseek(fp, 0, SEEK_SET);
//   int *buf = (int *)malloc(len * sizeof(int));
//   read_data(fp, buf, len);
//   std::sort(buf, buf + len - 1);
//   write_data(tmp, buf, len);
//   m_files_.enqueue(tmpfile);
//   fclose(fp);
//   fclose(tmp);
//   free(buf);
// }

// // 归并操作
// inline void ThreadPool::merge(const std::string &path1,
//                               const std::string &path2,
//                               const std::string &path3) {
//   FILE *fp1 = fopen(path1.c_str(), "r");
//   FILE *fp2 = fopen(path2.c_str(), "r");
//   FILE *fp3 = fopen(path3.c_str(), "wb+");
//   fseek(fp1, 0, SEEK_END);
//   int len1 = ftell(fp1);
//   fseek(fp1, 0, SEEK_SET);
//   fseek(fp2, 0, SEEK_END);
//   int len2 = ftell(fp2);
//   fseek(fp2, 0, SEEK_SET);
//   int *buf1 = (int *)malloc(len1);
//   int *buf2 = (int *)malloc(len2);
//   int *buf3 = (int *)malloc(len1 + len2);
//   read_data(fp1, buf1, len1);
//   read_data(fp2, buf2, len2);
//   int i = 0, j = 0, k = 0;
//   while (i < len1 && j < len2) {
//     if (buf1[i] < buf2[j])
//       buf3[k++] = buf1[i++];
//     else
//       buf3[k++] = buf2[j++];
//   }
//   while (i < len1) buf3[k++] = buf1[i++];
//   while (j < len2) buf3[k++] = buf2[j++];
//   write_data(fp3, buf3, len1 + len2);
//   m_files_.enqueue(path3);
//   fclose(fp1);
//   fclose(fp2);
//   fclose(fp3);
//   free(buf1);
//   free(buf2);
//   free(buf3);
// }
// /*
//  每次线程池从任务队列中取出一个任务执行
//  如果任务队列为空，则等待
// */
// inline void ThreadPool::process_task() {
//   while (true) {
//     Task task;
//     {
//       std::unique_lock<std::mutex> lock(this->m_mutex_);
//       // wait直到有task可以执行
//       this->m_cond_.wait(
//           lock, [this] { return this->m_stop_ || !this->m_tasks_.empty(); });
//       if (this->m_stop_ && this->m_tasks_.empty()) return;
//       this->m_tasks_.dequeue(task);
//     }
//     // 执行task
//     (task)();
//     std::cout << "Thread " << std::this_thread::get_id() << " finished task"
//               << std::endl;
//   }
// }

// inline void ThreadPool::merge_task(const std::string &file3) {
//   while (true) {
//     std::string file1, file2;
//     {
//       std::unique_lock<std::mutex> lock(this->m_mutex_);
//       // wait直到有task可以执行
//       this->m_cond_.wait(lock, [this] { return this->m_files_.size() == 2;
//       }); this->m_files_.dequeue(file1); this->m_files_.dequeue(file2);
//       m_tasks_.enqueue(
//           [this, file1, file2, file3] { this->merge(file1, file2, file3); });
//     }
//     // 执行task
//     std::cout << file1 << file2 << std::endl;
//     std::cout << "Thread " << std::this_thread::get_id() << " finished task"
//               << std::endl;
//   }
// }
// /*
//   构造函数，初始化线程池
//   参数：线程池中线程数量
// */
// inline ThreadPool::ThreadPool(size_t thread_num) : m_stop_(false) {
//   m_threadnum_ = thread_num < 1 ? 1 : thread_num;
//   for (size_t i = 1; i < thread_num; ++i) {
//     // 初始化执行的函数
//     m_workers_.emplace_back([this] { this->process_task(); });
//   }
//   std::cout << "Threads num: " << m_workers_.size() << std::endl;
// }

// /*
//   析构函数，销毁线程池
//   唤醒所有线程后，让每个线程退出
// */
// inline ThreadPool::~ThreadPool() {
//   {
//     // notify是进行原子的解锁和通知操作，需要加锁
//     std::unique_lock<std::mutex> lock(m_mutex_);
//     m_stop_ = true;
//   }
//   // 唤醒所有线程
//   m_cond_.notify_all();
//   // 令所有线程退出
//   for (std::thread &worker : m_workers_) {
//     worker.join();
//   }
//   std::cout << m_workers_.size() << std::endl;
// }

// /*
//   添加任务到任务队列
//   参数：任务函数，任务函数参数
//   返回值：任务函数的返回值
// */
// template <typename F, typename... Args>
// auto ThreadPool::enqueue_task(F &&f, Args &&...args)
//     -> std::future<typename std::result_of<F(Args...)>::type> {
//   using return_type = typename std::result_of<F(Args...)>::type;

//   // 如果线程池已经终止，则抛出异常
//   if (m_stop_.load()) {
//     throw std::runtime_error("enqueue on stopped ThreadPool");
//   }

//   auto task = std::make_shared<std::packaged_task<return_type()>>(
//       std::bind(std::forward<F>(f), std::forward<Args>(args)...));
//   std::future<return_type> res = task->get_future();
//   // 添加任务到队列
//   m_tasks_.enqueue([task]() { (*task)(); });
//   // 唤醒一个线程执行
//   std::lock_guard<std::mutex> lock(m_mutex_);
//   m_cond_.notify_one();
//   return res;
// }

// /*
//   添加文件到文件队列
//   参数：文件名
// */
// inline void ThreadPool::enqueue_file(std::string file) {
//   m_files_.enqueue(file);
// }
}  // namespace neo
#endif /* THREADPOOL_H_ */