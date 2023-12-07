#include "../include/CLOuterSort.h"

#include <string.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
namespace neo {
/*
最终的执行函数
*/
std::string OuterSort::run() {
  for (auto &file : m_need_files_) {
    enqueue_task([this, file] { this->separate_sort(file); });
  }
  while (1) {
    if (m_tasks_.size() == 0 && m_files_.size() == 1) {
      std::string s = m_files_.front();
      FILE *fp = fopen(s.c_str(), "rb");
      fseek(fp, 0, SEEK_END);
      long len = ftell(fp) / sizeof(uint64_t);
      if (s[0] == 'm' && m_num_ == len) {
        return s;
      }
    }
  }
  return nullptr;
}

/*
  分割大文件排序
  1. 如果文件小于内存限制，直接读取文件，排序后写入文件
  2. 如果文件超过内存限制，分块读取，排序后写入文件
*/
void OuterSort::separate_sort(const std::string &path) {
  FILE *fp = fopen(path.c_str(), "rb");
  std::string tmpfile = "t" + path;
  FILE *tmp = fopen(tmpfile.c_str(), "wb+");
  // 获取文件长度
  fseek(fp, 0, SEEK_END);
  long len = ftell(fp) / sizeof(uint64_t);
  // 增加文件数据的数量
  m_num_ += len;
  long tmplen = 0;
  // 如果文件超过内存限制，分块读取
  if (len > LIMITMEM / sizeof(uint64_t)) {
    int round = 1;
    /*
      每次读取LIMITMEM大小的数据，排序后写入文件
    */
    while (tmplen < len - LIMITMEM / sizeof(uint64_t)) {
      // 重新打开一个新文件存放拍好序的数据
      std::string ttmpfile = std::to_string(round) + tmpfile;
      FILE *ttmp = fopen(ttmpfile.c_str(), "wb+");
      round++;
      uint64_t *buf = (uint64_t *)malloc(LIMITMEM);
      // 读取数据，排序后写入文件数据
      fread(buf, sizeof(uint64_t), LIMITMEM / sizeof(uint64_t), fp);
      std::sort(buf, buf + LIMITMEM / sizeof(uint64_t));
      fwrite(buf, sizeof(uint64_t), LIMITMEM / sizeof(uint64_t), ttmp);
      tmplen += LIMITMEM / sizeof(uint64_t);
      // std::cout << "tmplen: " << tmplen << std::endl;
      // 将排序后的文件加入m_files
      enqueue_file(ttmpfile);
      if (m_files_.size() >= 2) {
        std::lock_guard<std::mutex> lock(m_file_mutex_);
        m_file_cond_.notify_one();
      }
      free(buf);
      fclose(ttmp);
    }
    if (tmplen >= len) return;
    /*
      读取小于内存限制的剩余数据，排序后写入文件
    */
    uint64_t *buf = (uint64_t *)malloc((len - tmplen) * sizeof(uint64_t));
    // 读取数据，排序后写入文件数据
    fread(buf, sizeof(uint64_t), len - tmplen, fp);
    std::sort(buf, buf + len - tmplen);
    fwrite(buf, sizeof(uint64_t), len - tmplen, tmp);
    // 将排序后的文件加入m_files
    enqueue_file(tmpfile);
    if (m_files_.size() >= 2) {
      std::lock_guard<std::mutex> lock(m_file_mutex_);
      m_file_cond_.notify_one();
    }
    free(buf);
  } else {
    /*
      如果文件小于内存限制，直接读取文件，排序后写入文件
    */
    uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));
    // 读取数据，排序后写入文件数据
    fread(buf, sizeof(uint64_t), len, fp);
    std::sort(buf, buf + len);
    fwrite(buf, sizeof(uint64_t), len, tmp);
    // 将排序后的文件加入m_files
    enqueue_file(tmpfile);
    if (m_files_.size() >= 2) {
      std::lock_guard<std::mutex> lock(m_file_mutex_);
      m_file_cond_.notify_one();
    }
    free(buf);
  }

  fclose(fp);
  fclose(tmp);
}

/*
  排序函数
  参数：文件名
*/
void OuterSort::sort(const std::string &path) {
  FILE *fp = fopen(path.c_str(), "rb");
  std::string tmpfile = "t" + path;
  FILE *tmp = fopen(tmpfile.c_str(), "wb+");
  FILE *fp1 = fopen(path.c_str(), "rb");
  // 获取文件长度
  fseek(fp1, 0, SEEK_END);
  int len = ftell(fp1) / sizeof(uint64_t);
  m_num_ += len;
  fseek(fp1, 0, SEEK_SET);
  uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));

  // 读取数据，排序后写入文件数据
  fread(buf, sizeof(uint64_t), len, fp);
  std::sort(buf, buf + len);
  fwrite(buf, sizeof(uint64_t), len, tmp);
  // 将排序后的文件加入m_files
  enqueue_file(tmpfile);
  // std::cout << "after sort " << m_files_.size() << std::endl;
  if (m_files_.size() >= 2) {
    std::lock_guard<std::mutex> lock(m_file_mutex_);
    m_file_cond_.notify_one();
  }
  fclose(fp);
  fclose(fp1);
  fclose(tmp);
  free(buf);
}

/*
  归并函数
  参数：两个文件名
*/
void OuterSort::merge(const std::string &path1, const std::string &path2) {
  std::string path3 = "m" + path1;
  std::ifstream fin1, fin2;
  fin1.open(path1.c_str(), std::ios::in | std::ios::binary);
  fin2.open(path2.c_str(), std::ios::in | std::ios::binary);
  std::ofstream fout;
  fout.open(path3.c_str(), std::ios::out | std::ios::binary);
  // 在每个文件中先各读出第一个数
  uint64_t temp1, temp2;
  fin1.read(reinterpret_cast<char *>(&temp1), sizeof(uint64_t));
  fin2.read(reinterpret_cast<char *>(&temp2), sizeof(uint64_t));
  // 在两个文件中前进，每次只选择两个文件中较小的那个数添加到输出文件的末尾
  while (true) {
    if (temp1 <= temp2) {
      fout.write(reinterpret_cast<const char *>(&temp1), sizeof(uint64_t));
      // 到文件的最后一个数读完之后，再读一次，保证每个文件中的所有数据都被读到。
      fin1.read(reinterpret_cast<char *>(&temp1), sizeof(uint64_t));
      if (fin1.eof()) {
        break;
      }
    } else {
      fout.write(reinterpret_cast<const char *>(&temp2), sizeof(uint64_t));
      fin2.read(reinterpret_cast<char *>(&temp2), sizeof(uint64_t));
      if (fin2.eof()) {
        break;
      }
    }
  }
  if (!fin1.eof()) {
    while (true) {
      // 另一个文件已经读完了，说明当前这个文件中最后读到的那个整数还没有被写出，所以要先写出。
      fout.write(reinterpret_cast<const char *>(&temp1), sizeof(uint64_t));
      fin1.read(reinterpret_cast<char *>(&temp1), sizeof(uint64_t));
      if (fin1.eof()) {
        break;
      }
    }
  } else {
    while (true) {
      fout.write(reinterpret_cast<const char *>(&temp2), sizeof(uint64_t));
      fin2.read(reinterpret_cast<char *>(&temp2), sizeof(uint64_t));
      if (fin2.eof()) {
        break;
      }
    }
  }
  // 将合并后的文件加入m_files
  enqueue_file(path3);
  // std::cout << "after merge " << m_files_.size() << std::endl;
  // long len = fout.tellp() / sizeof(uint64_t);
  // std::cout << "merge " << path1 << "+" << path2 << " len: " << len
  //           << " to: " << path3 << std::endl;

  if (m_files_.size() >= 2) {
    {
      std::lock_guard<std::mutex> lock(m_file_mutex_);
      m_file_cond_.notify_one();
    }
  }
  if (m_tasks_.empty() && m_files_.size() == 1) {
    std::lock_guard<std::mutex> lock(m_file_mutex_);
    m_file_cond_.notify_one();
  }

  fin1.close();
  fin2.close();
  fout.close();
}

/*
 每次线程池从任务队列中取出一个任务执行
 如果任务队列为空，则等待
*/
void OuterSort::process_task() {
  while (true) {
    Task task;
    {
      std::unique_lock<std::mutex> lock(this->m_mutex_);
      // wait直到有task可以执行
      this->m_cond_.wait(
          lock, [this] { return this->m_stop_ || !this->m_tasks_.empty(); });
      if (this->m_stop_.load() && this->m_tasks_.empty()) return;
      if (m_tasks_.size() == 0 && m_files_.size() == 1) {
        std::string s = m_files_.front();
        FILE *fp = fopen(s.c_str(), "rb");
        fseek(fp, 0, SEEK_END);
        long len = ftell(fp) / sizeof(uint64_t);
        if (s[0] == 'm' && m_num_ == len) {
          return;
        }
        continue;
      }
      this->m_tasks_.dequeue(task);
      // std::cout << "after process task: " << m_tasks_.size() << std::endl;
    }
    // 执行task
    (task)();
  }
}

/*
  合并线程函数：每次线程池从文件队列中取出两个文件进行合并
  如果文件队列中文件数量小于2，则等待

*/
void OuterSort::merge_task() {
  while (true) {
    std::string file1, file2;
    {
      std::unique_lock<std::mutex> lock(this->m_file_mutex_);

      // wait直到有m_files_中有超过两个文件可以合并
      this->m_file_cond_.wait(lock, [this] {
        return this->m_files_.size() >= 2 ||
               (m_tasks_.empty() && m_files_.size() == 1);
      });

      if (m_tasks_.size() == 0 && m_files_.size() == 1) {
        std::string s = m_files_.front();
        FILE *fp = fopen(s.c_str(), "rb");
        fseek(fp, 0, SEEK_END);
        long len = ftell(fp) / sizeof(uint64_t);
        if (s[0] == 'm' && m_num_ == len) {
          return;
        }
        continue;
      }
      if (m_stop_.load()) return;
      this->m_files_.dequeue(file1);
      this->m_files_.dequeue(file2);
      // std::cout << "after dequeue " << m_files_.size() << std::endl;
      // std::cout << " get two files to merge" << std::endl;
    }
    {
      std::lock_guard<std::mutex> tasklock(this->m_mutex_);
      /*
        此时分为两种情况：
        1. 如果此时任务队列为空，需要插入一个merge任务，然后唤醒一个线程执行
        2. 如果此时任务队列不为空，直接插入一个merge任务
      */
      if (m_tasks_.empty()) {
        m_tasks_.enqueue([this, file1, file2] { this->merge(file1, file2); });
        m_cond_.notify_one();
      } else {
        m_tasks_.enqueue([this, file1, file2] { this->merge(file1, file2); });
      }
      // std::cout << "after push merge task: " << m_tasks_.size() << std::endl;
    }
  }
}

/*
  构造函数，初始化线程池
  参数：线程池中线程数量
*/
OuterSort::OuterSort(size_t thread_num, std::vector<std::string> need_files)
    : m_num_(0), m_stop_(false), m_need_files_(need_files) {
  thread_num = thread_num < 1 ? 1 : thread_num;
  for (size_t i = 0; i < thread_num; ++i) {
    // 初始化执行的函数
    m_workers_.emplace_back([this] { this->process_task(); });
  }
  m_merge_worker_ = std::thread([this] { this->merge_task(); });
}

/*
  析构函数，销毁线程池
  唤醒所有线程后，让每个线程退出
*/
OuterSort::~OuterSort() {
  {
    // notify是进行原子的解锁和通知操作，需要加锁
    std::unique_lock<std::mutex> lock(m_mutex_);
    m_stop_ = true;

    // 唤醒所有线程
    m_cond_.notify_all();
  }
  // 令所有线程退出
  for (std::thread &worker : m_workers_) {
    worker.join();
  }
  m_merge_worker_.join();
}

/*
  添加任务到任务队列
  参数：任务函数，任务函数参数
  返回值：任务函数的返回值
*/
template <typename F, typename... Args>
auto OuterSort::enqueue_task(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;

  // 如果线程池已经终止，则抛出异常
  if (m_stop_.load()) {
    throw std::runtime_error("enqueue on stopped OuterSort");
  }

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  std::future<return_type> res = task->get_future();
  // 添加任务到队列
  m_tasks_.enqueue([task]() { (*task)(); });
  // 唤醒一个线程执行
  {
    std::lock_guard<std::mutex> lock(m_mutex_);
    m_cond_.notify_one();
  }
  return res;
}

/*
  添加文件到文件队列
  参数：文件名
*/
void OuterSort::enqueue_file(std::string file) { m_files_.enqueue(file); }
}  // namespace neo
