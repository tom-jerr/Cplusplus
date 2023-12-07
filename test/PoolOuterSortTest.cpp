#include <gtest/gtest.h>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../include/CLOuterSort.h"

void write_file(const std::string &path, int n) {
  uint64_t *a = (uint64_t *)malloc(n * sizeof(uint64_t));
  for (int i = 0; i < n; ++i) a[i] = rand() % 100000000;
  FILE *fp = fopen(path.c_str(), "wb+");
  fwrite(a, sizeof(uint64_t), n, fp);
  free(a);
  FILE *fp1 = fopen(path.c_str(), "rb");
  fseek(fp1, 0, SEEK_END);
  fclose(fp);
  fclose(fp1);
}

TEST(OuterSort, TwofilesTest) {
  // 两个文件各写入10000个随机数
  write_file("test1", 10000);
  write_file("test2", 10000);
  srand(time(NULL));
  std::vector<std::string> vec = {"test1", "test2"};
  neo::OuterSort outerSort(2, vec);
  std::string final2 = outerSort.run();

  FILE *fp = fopen(final2.c_str(), "rb");
  fseek(fp, 0, SEEK_END);
  long len = ftell(fp) / sizeof(uint64_t);
  fseek(fp, 0, SEEK_SET);

  std::cout << "len: " << len << std::endl;
  uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));
  fread(buf, sizeof(uint64_t), len, fp);
  for (int i = 0; i < len - 1; ++i) {
    ASSERT_LE(buf[i], buf[i + 1]);
  }
  ASSERT_EQ(len, 20000);
  free(buf);
}
int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}