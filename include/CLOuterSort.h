#ifndef CLOUTERSORT_H_
#define CLOUTERSORT_H_
#include <CLThreadPool.h>
#include <unistd.h>

#include <cstdio>
#include <string>
#include <vector>

class OuterSort {
 private:
  ThreadPool m_pool_;
  std::vector<std::string> m_files_;
  std::string m_respath_;
  std::vector<std::vector<int>> m_tmpsort_;

 private:
  void write_data(FILE *f, int a[], int n);
  void read_data(FILE *f, int a[], int n);
  void sort(const std::string &path, const std::string &tmpfile);
  void merge(const std::string &path1, const std::string &path2,
             const std::string &path3);

 public:
  OuterSort(const std::vector<std::string> &files, const std::string &respath,
            int threadnum);
  ~OuterSort() = default;
  void run();
};

// 向文本文件中写入数据
inline void OuterSort::write_data(FILE *f, int a[], int n) {
  for (int i = 0; i < n; ++i) fprintf(f, "%d ", a[i]);
}
// 从文本文件中读取数据
inline void OuterSort::read_data(FILE *f, int a[], int n) {
  for (int i = 0; i < n; ++i) fscanf(f, "%d", &a[i]);
}

inline OuterSort::OuterSort(const std::vector<std::string> &files,
                            const std::string &respath, int threadnum)
    : m_pool_(ThreadPool(threadnum)), m_files_(files), m_respath_(respath) {}

// 单个文件排序
inline void OuterSort::sort(const std::string &path,
                            const std::string &tmpfile) {
  FILE *fp = fopen(path.c_str(), "r+");
  FILE *tmp = fopen(tmpfile.c_str(), "wb+");
  fseek(fp, 0, SEEK_END);
  int len = ftell(fp) / sizeof(int);
  fseek(fp, 0, SEEK_SET);
  int *buf = (int *)malloc(len * sizeof(int));
  read_data(fp, buf, len);
  std::sort(buf, buf + len - 1);
  write_data(tmp, buf, len);
  fclose(fp);
  fclose(tmp);
  free(buf);
}

// 归并操作
inline void OuterSort::merge(const std::string &path1, const std::string &path2,
                             const std::string &path3) {
  FILE *fp1 = fopen(path1.c_str(), "r");
  FILE *fp2 = fopen(path2.c_str(), "r");
  FILE *fp3 = fopen(path3.c_str(), "wb+");
  fseek(fp1, 0, SEEK_END);
  int len1 = ftell(fp1);
  fseek(fp1, 0, SEEK_SET);
  fseek(fp2, 0, SEEK_END);
  int len2 = ftell(fp2);
  fseek(fp2, 0, SEEK_SET);
  int *buf1 = (int *)malloc(len1);
  int *buf2 = (int *)malloc(len2);
  int *buf3 = (int *)malloc(len1 + len2);
  read_data(fp1, buf1, len1);
  read_data(fp2, buf2, len2);
  int i = 0, j = 0, k = 0;
  while (i < len1 && j < len2) {
    if (buf1[i] < buf2[j])
      buf3[k++] = buf1[i++];
    else
      buf3[k++] = buf2[j++];
  }
  while (i < len1) buf3[k++] = buf1[i++];
  while (j < len2) buf3[k++] = buf2[j++];
  write_data(fp3, buf3, len1 + len2);
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
  free(buf1);
  free(buf2);
  free(buf3);
}

inline void OuterSort::run() {
  int filecnt = m_files_.size();
  int cnt = 0;
  // while (filecnt > 1) {
  //   std::vector<std::string> tmpfiles;
  //   for (int i = 0; i < filecnt; i++) {
  //     if (i + 1 < filecnt) {
  //       std::string tmpfile = "../data/tmp" + std::to_string(cnt++);
  //       tmpfiles.push_back(tmpfile);
  //       m_pool_.enqueue([this, i, tmpfile]() { sort(m_files_[i]); });
  //     } else {
  //       tmpfiles.push_back(m_files_[i]);
  //     }
  //   }
  //   // m_pool_.wait();
  //   m_files_ = tmpfiles;
  //   filecnt = m_files_.size();
  // }
  for (int i = 0; i < filecnt; i++) {
    std::string tmpfile = "../data/tmp" + std::to_string(cnt++);
    m_pool_.enqueue([this, i, tmpfile]() { this->sort(m_files_[i], tmpfile); });
  }
  sleep(1);  // wait for sort() done
  std::cout << "sort done\n";
  // std::string resfile = m_respath_;
  // m_pool_.enqueue(
  //     [this, resfile]() { merge(m_files_[0], m_files_[1], resfile); });
  // // m_pool_.wait();
  // std::cout << "done\n";
}
#endif /* CLOUTERSORT_H_ */