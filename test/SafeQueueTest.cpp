#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "../include/CLSafeQueue.h"

TEST(SafeQueueTest, MultiInsertTest) {
  neo::SafeQueue<int> queue;
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.push_back(std::thread([&queue, i]() {
      for (int j = 1; j <= 100; ++j) {
        queue.enqueue(i * 100 + j);
      }
    }));
  }
  for (auto &t : threads) {
    t.join();
  }
  ASSERT_EQ(queue.size(), 1000);
  for (int i = 0; i < 1000; ++i) {
    int val;
    queue.dequeue(val);
    ASSERT_GT(val, 0);
  }
  ASSERT_EQ(queue.size(), 0);
}

TEST(SafeQueueTest, MultiInsertRemoveTest) {
  neo::SafeQueue<int> queue;
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.push_back(std::thread([&queue, i]() {
      for (int j = 0; j < 100; ++j) {
        queue.enqueue(i * 100 + j);
      }
    }));
  }
  for (int i = 0; i < 10; ++i) {
    threads.push_back(std::thread([&queue]() {
      for (int j = 0; j < 100; ++j) {
        int val;
        queue.dequeue(val);
      }
    }));
  }
  for (auto &t : threads) {
    t.join();
  }
  ASSERT_EQ(queue.size(), 0);
}

int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}