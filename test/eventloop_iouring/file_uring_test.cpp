/**
 * @file file_uring_test.cpp
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

TEST(FileUringTest, SimpleWriteTest) {
  EventLoopThread loop_thread;
  EventLoop *loop = loop_thread.startLoop();

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
  // loop.loop();
}

// TEST(FileUringTest, SimpleReadTest) {
//   EventLoopThread loop_thread;
//   EventLoop *loop = loop_thread.startLoop();
//   FileUring file_uring{"test.txt", loop, false};
//   const char *str = "Hello, io_uring!";
//   char buffer1[17], buffer2[17], buffer3[17];
//   size_t len = strlen(str);

//   auto future1 = file_uring.asyncRead(buffer1, len, 0, [&]() {
//     // buffer1[len] = '\0';
//     LOG_INFO << "读取1完成: " << buffer1;
//     EXPECT_STREQ(buffer1, str);
//   });
//   auto future2 = file_uring.asyncRead(buffer2, len, len, [&]() {
//     // buffer2[len] = '\0';
//     LOG_INFO << "读取2完成: " << buffer2;
//     EXPECT_STREQ(buffer2, str);
//   });
//   auto future3 = file_uring.asyncRead(buffer3, len, 2 * len, [&]() {
//     // buffer3[len] = '\0';
//     LOG_INFO << "读取3完成: " << buffer3;
//     EXPECT_STREQ(buffer3, str);
//   });

//   bool io1 = future1.get();
//   EXPECT_EQ(io1, true);
//   bool io2 = future2.get();
//   EXPECT_EQ(io2, true);
//   bool io3 = future3.get();
//   EXPECT_EQ(io3, true);
//   // loop.loop();
// }

// TEST(FileUringTest, SequentialWriteReadTest) {
//   // EventLoop loop;
//   EventLoopThread loop_thread;
//   EventLoop *loop = loop_thread.startLoop();
//   FileUring file_uring{"test2.txt", loop, false};
//   const char *str = "Hello, io_uring!";
//   char buffer[17];
//   size_t len = strlen(str);

//   auto future1 =
//       file_uring.asyncWrite(static_cast<void *>(const_cast<char *>(str)),
//       len,
//                             0, [&]() { LOG_INFO << "写入完成"; });
//   bool io1 = future1.get();
//   EXPECT_EQ(io1, true);
//   auto future2 = file_uring.asyncRead(buffer, len, 0, [&]() {
//     LOG_INFO << "读取完成: " << buffer;
//     EXPECT_STREQ(buffer, str);
//   });
//   bool io2 = future2.get();
//   EXPECT_EQ(io2, true);
//   // loop.loop();
// }

// TEST(FileUringTest, MultiWriteTest) {
//   EventLoopThread loop_thread;
//   EventLoop *loop = loop_thread.startLoop();
//   FileUring file_uring{"test3.txt", loop, false};
//   std::vector<std::future<bool>> futures;
//   std::vector<std::string> strings;
//   // 构造要写入的字符串，根据 i 来生成不同的字符串
//   for (int i = 0; i < 10; ++i) {
//     std::string str = "Hello, io_uring! " + std::to_string(i);
//     strings.emplace_back(str);
//   }
//   // 异步写入字符串
//   for (size_t i = 0; i < strings.size(); ++i) {
//     char *str = const_cast<char *>(strings[i].c_str());
//     size_t len = strings[i].length();
//     auto future = file_uring.asyncWrite(static_cast<void *>(str), len,
//                                         static_cast<size_t>(i) * len, [&]() {
//                                           // donothing
//                                           printf("Write finished\n");
//                                           // EXPECT_STREQ(str,
//                                           // strings[i].c_str());
//                                         });
//     futures.emplace_back(std::move(future));
//   }
//   // 等待所有异步写入操作完成
//   for (auto &future : futures) {
//     bool io = future.get();
//     EXPECT_EQ(io, true);
//   }
// }

// TEST(FileUringTest, MultiReadTest) {
//   EventLoopThread loop_thread;
//   EventLoop *loop = loop_thread.startLoop();
//   FileUring file_uring{"test3.txt", loop, false};
//   std::vector<std::future<bool>> futures;
//   std::vector<std::string> strings;
//   std::vector<char *> buffers;
//   // 构造要读取的字符串，根据 i 来生成不同的字符串
//   for (int i = 0; i < 10; ++i) {
//     std::string str = "Hello, io_uring! " + std::to_string(i);
//     strings.emplace_back(str);
//     char *buffer = new char[str.length() + 1];
//     buffers.emplace_back(buffer);
//     memset(buffer, 0, str.length() + 1);
//   }
//   // 异步读取字符串
//   for (size_t i = 0; i < strings.size(); ++i) {

//     size_t len = strings[i].length();
//     auto future =
//         file_uring.asyncRead(buffers[i], len, static_cast<size_t>(i) * len,
//                              [&]() { LOG_INFO << "读取完成"; });
//     futures.emplace_back(std::move(future));
//   }
//   // 等待所有异步读取操作完成
//   for (auto &future : futures) {
//     bool io = future.get();
//     EXPECT_EQ(io, true);
//   }
//   // evaluate the read results
//   for (size_t i = 0; i < strings.size(); ++i) {
//     LOG_INFO << "读取结果: " << buffers[i];
//     EXPECT_STREQ(buffers[i], strings[i].c_str());
//   }
//   // 释放内存
//   for (auto buffer : buffers) {
//     delete[] buffer;
//   }
// }