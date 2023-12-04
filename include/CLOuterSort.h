#ifndef CLOUTERSORT_H_
#define CLOUTERSORT_H_
#include <CLThreadPool.h>
#include <unistd.h>

#include <cstdio>
#include <string>
#include <vector>

namespace neo {
// 向文本文件中写入数据
inline void write_data(FILE *f, int a[], int n) {
  for (int i = 0; i < n; ++i) fprintf(f, "%d ", a[i]);
}
// 从文本文件中读取数据
inline void read_data(FILE *f, int a[], int n) {
  for (int i = 0; i < n; ++i) fscanf(f, "%d", &a[i]);
}

// 单个文件排序
inline void sort(const std::string &path, const std::string &tmpfile) {
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
inline void merge(const std::string &path1, const std::string &path2,
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

}  // namespace neo
#endif /* CLOUTERSORT_H_ */