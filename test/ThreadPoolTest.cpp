#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include "../include/CLThreadPool.h"

// void first(int id) {
//   std::cout << "hello from " << id << ", function\n";
//   std::this_thread::sleep_for(std::chrono::seconds(1));
// }

// void aga(int id, int par) {
//   std::cout << "hello from " << id << ", function with parameter " << par
//             << '\n';
// }

// void mmm(int id, const std::string &s) {
//   std::cout << "mmm function " << id << ' ' << s << '\n';
// }
void write_data(FILE *f, int a[], int n) {
  for (int i = 0; i < n; ++i) fprintf(f, "%d ", a[i]);
}

void read_data(FILE *f, int a[], int n) {
  for (int i = 0; i < n; ++i) fscanf(f, "%d", &a[i]);
}

void writefile(const std::string &path) {
  FILE *fp = fopen(path.c_str(), "w+");

  for (int i = 1; i <= 1000; i++) {
    fprintf(fp, "%d ", i);
  }
  std::cout << "write file done\n";
  fclose(fp);
}

void sort(const std::string &path, const std::string &path2) {
  FILE *fp = fopen(path.c_str(), "r+");
  FILE *fp2 = fopen(path2.c_str(), "w+");
  fseek(fp, 0, SEEK_END);
  int len = ftell(fp) / sizeof(int);
  std::cout << len << '\n';
  fseek(fp, 0, SEEK_SET);
  int *buf = (int *)malloc(1000 * sizeof(int));
  read_data(fp, buf, len);
  std::sort(buf, buf + len - 1);
  write_data(fp2, buf, len);
  fclose(fp);
  fclose(fp2);
  //   std::cout << "sort from " << begin << " to " << end << '\n';
}
void merge(const std::string &path1, const std::string &path2,
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
  int *buf1 = (int *)malloc(1000 * sizeof(int));
  int *buf2 = (int *)malloc(1000 * sizeof(int));
  len1 = len2 = 1000;
  read_data(fp1, buf1, len1);
  read_data(fp2, buf2, len2);
  int i = 0, j = 0;
  while (i < len1 && j < len2) {
    if (buf1[i] < buf2[j]) {
      fprintf(fp3, "%d ", buf1[i]);
      ++i;
    } else {
      fprintf(fp3, "%d ", buf2[j]);
      ++j;
    }
  }
  while (i < len1) {
    fprintf(fp3, "%d ", buf1[i]);
    ++i;
  }
  while (j < len2) {
    fprintf(fp3, "%d ", buf2[j]);
    ++j;
  }
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
}

int main(int argc, char **argv) {
  ThreadPool p(2 /* two threads in the pool */);
  p.enqueue(writefile, "test.txt");
  p.enqueue(writefile, "test2.txt");
  // sleep(1);
  p.enqueue(sort, "test.txt", "tmp1.txt");
  p.enqueue(sort, "test2.txt", "tmp2.txt");
  // sleep(1);
  p.enqueue(merge, "tmp1.txt", "tmp2.txt", "test3.txt");
  // p.enqueue(first, 1);   // function
  // p.enqueue(aga, 7, 2);  // function

  // p.enqueue(mmm, 3, "worked");

  return 0;
}