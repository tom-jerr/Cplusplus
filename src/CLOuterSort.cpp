#include "../include/CLOuterSort.h"

#include <iostream>
namespace neo {
/*
最终的执行函数
*/
std::string OuterSort::run() {
  for (auto &file : m_need_files_) {
    enqueue_task([this, file] { this->sort(file); });
  }
  while (1) {
    if (m_tasks_.size() == 0 && m_files_.size() == 1) {
      std::string s = m_files_.front();
      if (s[0] == 'm') {
        return s;
      }
    }
  }
  return nullptr;
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
  fseek(fp1, 0, SEEK_SET);
  uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));

  // 读取数据，排序后写入文件数据
  fread(buf, sizeof(uint64_t), len, fp);
  std::sort(buf, buf + len);
  fwrite(buf, sizeof(uint64_t), len, tmp);
  // 将排序后的文件加入m_files
  enqueue_file(tmpfile);
  std::cout << "after sort " << m_files_.size() << std::endl;
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
  FILE *fp1 = fopen(path1.c_str(), "rb");
  FILE *fp2 = fopen(path2.c_str(), "rb");
  std::string path3 = "m" + path1;
  FILE *fp3 = fopen(path3.c_str(), "wb+");
  // 获取文件长度
  fseek(fp1, 0, SEEK_END);
  long len1 = ftell(fp1) / sizeof(uint64_t);
  fseek(fp1, 0, SEEK_SET);

  fseek(fp2, 0, SEEK_END);
  long len2 = ftell(fp2) / sizeof(uint64_t);
  fseek(fp2, 0, SEEK_SET);
  // 为读取的数据分配空间
  uint64_t *buf1 = (uint64_t *)malloc(len1 * sizeof(uint64_t));
  uint64_t *buf2 = (uint64_t *)malloc(len2 * sizeof(uint64_t));
  uint64_t *buf3 = (uint64_t *)malloc((len1 + len2) * sizeof(uint64_t));
  // 读取文件合并后写回文件
  fread(buf1, sizeof(uint64_t), len1, fp1);
  fread(buf2, sizeof(uint64_t), len2, fp2);
  int i = 0, j = 0, k = 0;
  while (i < len1 && j < len2) {
    if (buf1[i] < buf2[j])
      buf3[k++] = buf1[i++];
    else
      buf3[k++] = buf2[j++];
  }
  while (i < len1) buf3[k++] = buf1[i++];
  while (j < len2) buf3[k++] = buf2[j++];
  fwrite(buf3, sizeof(uint64_t), len1 + len2, fp3);

  // 将合并后的文件加入m_files
  enqueue_file(path3);
  std::cout << "after merge " << m_files_.size() << std::endl;
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
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
  free(buf1);
  free(buf2);
  free(buf3);
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
      if (this->m_tasks_.empty() && m_files_.size() == 1) {
        std::string s = m_files_.front();
        if (s[0] == 'm') {
          return;
        }
        continue;
      }
      this->m_tasks_.dequeue(task);
      std::cout << "after process task: " << m_tasks_.size() << std::endl;
    }
    // 执行task
    (task)();
    // std::cout << "Thread " << std::this_thread::get_id() << " finished
    // task"
    //           << std::endl;
  }
}

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
        if (s[0] == 'm') {
          break;
        }
        continue;
      }
      if (m_stop_.load()) return;
      this->m_files_.dequeue(file1);
      this->m_files_.dequeue(file2);
      std::cout << "after dequeue " << m_files_.size() << std::endl;
      std::cout << " get two files to merge" << std::endl;
    }
    {
      std::lock_guard<std::mutex> tasklock(this->m_mutex_);
      if (m_tasks_.empty()) {
        m_tasks_.enqueue([this, file1, file2] { this->merge(file1, file2); });
        m_cond_.notify_one();
      } else {
        m_tasks_.enqueue([this, file1, file2] { this->merge(file1, file2); });
      }
      std::cout << "after push merge task: " << m_tasks_.size() << std::endl;
    }
  }
}

/*
  构造函数，初始化线程池
  参数：线程池中线程数量
*/
OuterSort::OuterSort(size_t thread_num, std::vector<std::string> need_files)
    : m_stop_(false), m_need_files_(need_files) {
  m_threadnum_ = thread_num < 1 ? 1 : thread_num;

  for (size_t i = 0; i < thread_num; ++i) {
    // 初始化执行的函数
    m_workers_.emplace_back([this] { this->process_task(); });
  }
  m_merge_worker_ = std::thread([this] { this->merge_task(); });
  // std::cout << "Threads num: " << m_workers_.size() << std::endl;
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
  // std::cout << m_workers_.size() << std::endl;
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

void write_file(const std::string &path, int n) {
  uint64_t *a = (uint64_t *)malloc(n * sizeof(uint64_t));
  for (int i = 0; i < n; ++i) a[i] = rand() % 1000000;
  FILE *fp = fopen(path.c_str(), "wb+");
  fwrite(a, sizeof(uint64_t), n, fp);
  free(a);
  FILE *fp1 = fopen(path.c_str(), "rb");
  fseek(fp1, 0, SEEK_END);
  // int len = ftell(fp1);
  fclose(fp);
  fclose(fp1);
}

int main() {
  // testing::InitGoogleTest();
  // return RUN_ALL_TESTS();
  write_file("test1", 1000);
  write_file("test2", 1000);
  srand(time(NULL));
  std::vector<std::string> vec = {"test1", "test2"};
  neo::OuterSort outerSort(2, vec);
  std::string final2 = outerSort.run();
  std::cout << final2 << std::endl;
  //  sleep(3);
  // std::string final = outerSort.getfinalfile();
  std::string final = "mttest2";
  FILE *fp1 = fopen(final2.c_str(), "rb");
  fseek(fp1, 0, SEEK_END);
  int len = ftell(fp1) / sizeof(uint64_t);
  // std::cout << len << std::endl;
  uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));
  fread(buf, sizeof(uint64_t), len, fp1);
  for (int i = 0; i < len - 1; ++i) {
    if (buf[i] > buf[i + 1]) {
      std::runtime_error("sort error\n");
    }
  }

  // ASSERT_EQ(len, 1000);
  fclose(fp1);
  free(buf);
}