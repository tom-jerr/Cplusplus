// #include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "../include/CLOuterSort.h"

// TEST(OuterSortTest, sort) {
//   srand(time(NULL));
//   std::vector<std::string> vec = {"test1", "test2"};
//   neo::OuterSort outerSort(2, vec);
//   outerSort.run();
//   sleep(3);
//   // std::string final = outerSort.getfinalfile();
//   std::string final = "mttest2";
//   FILE *fp1 = fopen(final.c_str(), "rb");
//   fseek(fp1, 0, SEEK_END);
//   int len = ftell(fp1) / sizeof(uint64_t);
//   // std::cout << len << std::endl;
//   uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));
//   fread(buf, sizeof(uint64_t), len, fp1);
//   for (int i = 0; i < len - 1; ++i) {
//     EXPECT_LE(buf[i], buf[i + 1]);
//   }

//   // ASSERT_EQ(len, 1000);
//   fclose(fp1);
//   free(buf);
// }

int main() {
  // testing::InitGoogleTest();
  // return RUN_ALL_TESTS();
  srand(time(NULL));
  std::vector<std::string> vec = {"test1", "test2"};
  neo::OuterSort outerSort(2, vec);
  outerSort.run();
  sleep(3);
  // std::string final = outerSort.getfinalfile();
  std::string final = "mttest2";
  FILE *fp1 = fopen(final.c_str(), "rb");
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