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
// TODO(LZY):
// 需要提供 future promise 机制来通知异步操作完成
TEST(FileUringTest, MultiReadTest) {
  // EventLoop loop;
  EventLoopThread loop_thread;
  EventLoop *loop = loop_thread.startLoop();
  FileUring file_uring{"test.txt", loop, false};
  const char *str = "Hello, io_uring!";
  char buffer1[17], buffer2[17], buffer3[17];
  size_t len = strlen(str);

  auto future1 = file_uring.asyncRead(buffer1, len, 0, [&]() {
    // buffer1[len] = '\0';
    LOG_INFO << "读取1完成: " << buffer1;
    EXPECT_STREQ(buffer1, str);
  });
  auto future2 = file_uring.asyncRead(buffer2, len, len, [&]() {
    // buffer2[len] = '\0';
    LOG_INFO << "读取2完成: " << buffer2;
    EXPECT_STREQ(buffer2, str);
  });
  auto future3 = file_uring.asyncRead(buffer3, len, 2 * len, [&]() {
    // buffer3[len] = '\0';
    LOG_INFO << "读取3完成: " << buffer3;
    EXPECT_STREQ(buffer3, str);
  });

  bool io1 = future1.get();
  EXPECT_EQ(io1, true);
  bool io2 = future2.get();
  EXPECT_EQ(io2, true);
  bool io3 = future3.get();
  EXPECT_EQ(io3, true);
  // loop.loop();
}

// TEST(FileUringTest, SequentialWriteReadTest) {
//   // EventLoop loop;
//   EventLoopThread loop_thread;
//   EventLoop *loop = loop_thread.startLoop();
//   FileUring file_uring{"test2.txt", loop, false};
//   const char *str = "Hello, io_uring!";
//   char buffer[17];
//   size_t len = strlen(str);

//   file_uring.asyncWrite(static_cast<void *>(const_cast<char *>(str)), len, 0,
//                         [&]() {
//                           LOG_INFO << "写入完成";
//                           // file_uring.asyncRead(buffer, len, 0, [&]() {
//                           //   LOG_INFO << "读取完成: " << buffer;
//                           //   EXPECT_STREQ(buffer, str);
//                           // });
//                         });
//   sleep(2);
//   file_uring.asyncRead(buffer, len, 0, [&]() {
//     LOG_INFO << "读取完成: " << buffer;
//     EXPECT_STREQ(buffer, str);
//   });
//   sleep(2);
//   // loop.loop();
// }
// TEST(FileUringTest, IOChainWriteReadTest) {
//   // EventLoop loop;
//   EventLoopThread loop_thread;
//   EventLoop *loop = loop_thread.startLoop();
//   FileUring file_uring{"test2.txt", loop, false};
//   const char *str = "Hello, io_uring!";
//   char buffer[17];
//   size_t len = strlen(str);

//   file_uring.asyncWrite(static_cast<void *>(const_cast<char *>(str)), len, 0,
//                         [&]() {
//                           LOG_INFO << "写入完成";
//                           file_uring.asyncRead(buffer, len, 0, [&]() {
//                             LOG_INFO << "读取完成: " << buffer;
//                             EXPECT_STREQ(buffer, str);
//                           });
//                         });
//   sleep(2);
//   // loop.loop();
// }