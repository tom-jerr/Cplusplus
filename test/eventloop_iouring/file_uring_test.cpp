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
// loop.loop()后，整个测试会卡住，需要使用其他线程来进行loop，主线程执行IO时唤醒即可
TEST(FileUringTest, SimpleTest) {
  EventLoop loop;
  FileUring file_uring{"test.txt", &loop, false};
  const char *str = "Hello, io_uring!";
  char buffer[1024];
  size_t len = strlen(str);

  // file_uring.asyncWrite(static_cast<void *>(const_cast<char *>(str)), len, 0,
  //                       [&]() { LOG_INFO << "写入完成"; });
  file_uring.asyncRead(buffer, len, 0, [&]() {
    auto content = new char[len + 1];
    memcpy(content, buffer, len);
    content[len] = '\0';
    LOG_INFO << "读取1完成: " << content;
    EXPECT_STREQ(buffer, str);
    delete[] content;
  });
  file_uring.asyncRead(buffer, len, len, [&]() {
    auto content = new char[len + 1];
    memcpy(content, buffer + len, len);
    content[len] = '\0';
    LOG_INFO << "读取2完成: " << content;
    EXPECT_STREQ(buffer, str);
    delete[] content;
  });
  file_uring.asyncRead(buffer, len, 2 * len, [&]() {
    auto content = new char[len + 1];
    memcpy(content, buffer + 2 * len, len);
    content[len] = '\0';
    LOG_INFO << "读取3完成: " << content;
    EXPECT_STREQ(buffer, str);
    delete[] content;
  });
  loop.loop();
}

TEST(FileUringTest, CombineCallbackTest) {
  EventLoop loop;
  FileUring file_uring{"test.txt", &loop, false};
  const char *str = "Hello, io_uring!";
  char buffer[1024];
  size_t len = strlen(str);

  file_uring.asyncWrite(static_cast<void *>(const_cast<char *>(str)), len, 0,
                        [&]() {
                          LOG_INFO << "写入完成";
                          file_uring.asyncRead(buffer, len, 0, [&]() {
                            LOG_INFO << "读取完成: " << buffer;
                            EXPECT_STREQ(buffer, str);
                          });
                        });
  loop.loop();
}