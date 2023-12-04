#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>

#include "../include/CLUtils.h"

void write_file(const std::string &path, int n) {
  uint64_t *a = (uint64_t *)malloc(n * sizeof(uint64_t));
  for (int i = 0; i < n; ++i) a[i] = rand() % 1000000;
  FILE *fp = fopen(path.c_str(), "wb+");
  fwrite(a, sizeof(uint64_t), n, fp);
  free(a);
  FILE *fp1 = fopen(path.c_str(), "rb");
  fseek(fp1, 0, SEEK_END);
  int len = ftell(fp1);
  // std::cout << len << std::endl;
  // std::cout << sizeof(uint64_t) << std::endl;
  fclose(fp);
  fclose(fp1);
}

TEST(UtilsTest, sort) {
  srand(time(NULL));
  write_file("test2", 1000);
  neo::Utils utils;
  std::string path = "test2";
  std::string tmpfile = "tmp_utils2";
  utils.sort(path, tmpfile);
  FILE *fp1 = fopen(tmpfile.c_str(), "rb");
  fseek(fp1, 0, SEEK_END);
  int len = ftell(fp1) / sizeof(uint64_t);
  // std::cout << len << std::endl;
  uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));
  fread(buf, sizeof(uint64_t), len, fp1);
  for (int i = 0; i < len - 1; ++i) {
    EXPECT_LE(buf[i], buf[i + 1]);
  }

  ASSERT_EQ(len, 1000);
  fclose(fp1);
  free(buf);
}

int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}