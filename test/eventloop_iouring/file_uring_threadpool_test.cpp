/**
 * @file file_uring_threadpool_test.cpp
 * @author lzy
 * @brief
 * @version 0.1
 * @date 2025-04-23
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/EventLoopThreadPool.h"
#include "muduo/net/iouringfile/FileUring.h"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <liburing.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>
#define ENTRIES 1024 // io_uring 队列大小
#define BATCH_SIZE 16
using namespace muduo;
using namespace muduo::net;

TEST(FileUringThreadPoolTest, SingleLoopWriteTest) {
  // 初始化代码
  EventLoop base_loop;
  EventLoopThreadPool thread_pool(&base_loop, "test_threadpool");
  thread_pool.setThreadNum(4);
  thread_pool.start();

  auto *loop = thread_pool.getNextLoop();

  FileUring file_uring{"test.txt", loop, false};
  const char *str = "Hello, io_uring!";
  size_t len = strlen(str);

  auto future = file_uring.asyncWrite(
      static_cast<void *>(const_cast<char *>(str)), len, 0, [&]() {
        LOG_INFO << "写入完成";
        EXPECT_EQ(len, strlen(str));
      });

  bool io = future.get();
  EXPECT_EQ(io, true);

  auto future2 = file_uring.asyncWrite(
      static_cast<void *>(const_cast<char *>(str)), len, len, [&]() {
        LOG_INFO << "写入完成";
        EXPECT_EQ(len, strlen(str));
      });
  bool io2 = future2.get();
  EXPECT_EQ(io2, true);
  auto future3 = file_uring.asyncWrite(
      static_cast<void *>(const_cast<char *>(str)), len, 2 * len, [&]() {
        LOG_INFO << "写入完成";
        EXPECT_EQ(len, strlen(str));
      });
  bool io3 = future3.get();
  EXPECT_EQ(io3, true);

  // read from file
  char buffer[17];
  for (int i = 0; i < 3; ++i) {
    auto future4 =
        file_uring.asyncRead(buffer, len, static_cast<size_t>(i) * len, [&]() {
          buffer[len] = '\0';
          LOG_INFO << "读取完成: " << buffer;
          EXPECT_STREQ(buffer, str);
        });
    bool io4 = future4.get();
    EXPECT_EQ(io4, true);
  }
}

TEST(FileUringThreadPoolTest, MultiLoopWriteTest) {
  // 初始化代码
  EventLoop base_loop;
  EventLoopThreadPool thread_pool(&base_loop, "test_threadpool");
  thread_pool.setThreadNum(4);
  thread_pool.start();

  auto *loop1 = thread_pool.getNextLoop();
  auto *loop2 = thread_pool.getNextLoop();
  auto *loop3 = thread_pool.getNextLoop();

  FileUring file_uring1{"pool_test1.txt", loop1, false};
  FileUring file_uring2{"pool_test2.txt", loop2, false};
  FileUring file_uring3{"pool_test3.txt", loop3, false};
  const char *str = "Hello, io_uring!";
  size_t len = strlen(str);
  std::vector<std::future<bool>> futures;

  // write every file str 3 times
  for (int i = 0; i < 3; ++i) {
    auto future1 =
        file_uring1.asyncWrite(static_cast<void *>(const_cast<char *>(str)),
                               len, static_cast<size_t>(i) * len, [&]() {
                                 LOG_INFO << "写入完成";
                                 EXPECT_EQ(len, strlen(str));
                               });
    futures.emplace_back(std::move(future1));

    auto future2 =
        file_uring2.asyncWrite(static_cast<void *>(const_cast<char *>(str)),
                               len, static_cast<size_t>(i) * len, [&]() {
                                 LOG_INFO << "写入完成";
                                 EXPECT_EQ(len, strlen(str));
                               });
    futures.emplace_back(std::move(future2));

    auto future3 =
        file_uring3.asyncWrite(static_cast<void *>(const_cast<char *>(str)),
                               len, static_cast<size_t>(i) * len, [&]() {
                                 LOG_INFO << "写入完成";
                                 EXPECT_EQ(len, strlen(str));
                               });
    futures.emplace_back(std::move(future3));
  }

  // 等待所有异步写入操作完成
  for (auto &future : futures) {
    bool io = future.get();
    EXPECT_EQ(io, true);
  }

  // read from three files
  char buffer1[17];
  char buffer2[17];
  char buffer3[17];
  for (int i = 0; i < 3; ++i) {

    auto future1 = file_uring1.asyncRead(buffer1, len, 0, [&]() {
      buffer1[len] = '\0';
      LOG_INFO << "读取完成: " << buffer1;
      EXPECT_STREQ(buffer1, str);
    });
    auto future2 = file_uring2.asyncRead(buffer2, len, 0, [&]() {
      buffer2[len] = '\0';
      LOG_INFO << "读取完成: " << buffer2;
      EXPECT_STREQ(buffer2, str);
    });
    auto future3 = file_uring3.asyncRead(buffer3, len, 0, [&]() {
      buffer3[len] = '\0';
      LOG_INFO << "读取完成: " << buffer3;
      EXPECT_STREQ(buffer3, str);
    });
    bool io1 = future1.get();
    EXPECT_EQ(io1, true);
    bool io2 = future2.get();
    EXPECT_EQ(io2, true);
    bool io3 = future3.get();
    EXPECT_EQ(io3, true);
  }
}
