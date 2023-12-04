#include <gtest/gtest.h>

#include <cstdio>

#include "CLOuterSort.h"
#include "CLThreadPool.h"

FILE *fp1 = fopen("../data/1.txt", "r");
FILE *fp2 = fopen("../data/2.txt", "r");
TEST(SortTest, SingleSortTest) {
  neo::ThreadPool pool(2);
  pool.enqueue_task(neo::sort, "../data/1.txt", "../data/1.tmp");
  pool.enqueue_task(neo::sort, "../data/2.txt", "../data/2.tmp");
  // pool.enqueue_task(neo::merge, "../data/1.tmp", "../data/2.tmp",
  //                   "../data/3.txt");
}

int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}