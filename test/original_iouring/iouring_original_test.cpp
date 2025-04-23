/**
 * @file iouring_test.cpp
 * @author lzy
 * @brief
 * @version 0.1
 * @date 2025-04-23
 *
 * @copyright Copyright (c) 2025
 *
 */
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

class IoUringTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化 io_uring 实例
    int ret = io_uring_queue_init(ENTRIES, &ring_, 0);
    if (ret < 0) {
      fprintf(stderr, "io_uring 初始化失败: %s\n", strerror(-ret));
      exit(1);
    }
  }

  void TearDown() override { io_uring_queue_exit(&ring_); }

  struct io_uring ring_;
};
TEST_F(IoUringTest, BasicTest) {
  int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd, -1) << "文件打开失败";

  const char *str = "Hello, io_uring!";
  size_t len = strlen(str);

  // 提交 BATCH_SIZE 个写操作
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe) << "无法获取 SQE";
    io_uring_prep_write(sqe, fd, str, static_cast<unsigned int>(len), 0);
    sqe->user_data = static_cast<__U64_TYPE>(i + 1);
  }

  // 提交所有 SQEs 到内核
  int ret = io_uring_submit(&ring_);
  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);

  close(fd);
}

TEST_F(IoUringTest, UserDataBasicTest) {
  int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd, -1) << "文件打开失败";

  const char *str = "Hello, io_uring!";
  size_t len = strlen(str);

  // 提交 BATCH_SIZE 个写操作
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe) << "无法获取 SQE";
    io_uring_prep_write(sqe, fd, str, static_cast<unsigned int>(len), 0);
    sqe->user_data = static_cast<__U64_TYPE>(i + 1);
  }

  // 提交所有 SQEs 到内核
  int ret = io_uring_submit(&ring_);
  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);

  // 检查用户数据是否正确
  struct io_uring_cqe *cqe;
  for (int i = 0; i < BATCH_SIZE; ++i) {
    ret = io_uring_wait_cqe(&ring_, &cqe);
    ASSERT_EQ(ret, 0) << "等待 CQE 时出错: " << strerror(-ret);
    EXPECT_EQ(cqe->user_data, static_cast<__U64_TYPE>(i + 1));
    io_uring_cqe_seen(&ring_, cqe);
  }

  close(fd);
}
TEST_F(IoUringTest, UserDataPtrTest) {
  int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd, -1) << "文件打开失败";
  const char *str = "Hello, io_uring!";
  const char *str_test = "test";
  size_t len = strlen(str);

  // 提交 BATCH_SIZE 个写操作
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe) << "无法获取 SQE";
    io_uring_prep_write(sqe, fd, str, static_cast<unsigned int>(len), 0);
    sqe->user_data = reinterpret_cast<__U64_TYPE>(str_test);
  }

  // 提交所有 SQEs 到内核
  int ret = io_uring_submit(&ring_);
  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);

  // 检查用户数据是否正确
  struct io_uring_cqe *cqe;
  for (int i = 0; i < BATCH_SIZE; ++i) {
    ret = io_uring_wait_cqe(&ring_, &cqe);
    ASSERT_EQ(ret, 0) << "等待 CQE 时出错: " << strerror(-ret);
    EXPECT_EQ(reinterpret_cast<const char *>(cqe->user_data), "test");
    io_uring_cqe_seen(&ring_, cqe);
  }

  close(fd);
}

TEST_F(IoUringTest, FirstWriteSecondReadTest) {
  int fd = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd, -1) << "文件打开失败";

  const char *str = "Hello, io_uring!";
  size_t len = strlen(str);

  // 提交 BATCH_SIZE 个写操作
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe) << "无法获取 SQE";
    io_uring_prep_write(sqe, fd, str, static_cast<unsigned int>(len), 0);
    sqe->user_data = static_cast<__U64_TYPE>((i + 1) << 1 | 1);
  }

  // 提交所有 SQEs 到内核
  int ret = io_uring_submit(&ring_);
  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);

  // 提交 BATCH_SIZE 个读操作
  char buffer[BATCH_SIZE][128];
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe) << "无法获取 SQE";
    io_uring_prep_read(sqe, fd, buffer[i], sizeof(buffer[i]), 0);
    sqe->user_data = static_cast<__U64_TYPE>((i + 1) << 1 | 0);
  }

  // 提交所有 SQEs 到内核
  ret = io_uring_submit(&ring_);
  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);

  // 检查用户数据是否正确，并验证读取的数据
  struct io_uring_cqe *cqe;

  for (int i = 0; i < 2 * BATCH_SIZE; ++i) {
    ret = io_uring_wait_cqe(&ring_, &cqe);
    ASSERT_EQ(ret, 0) << "等待 CQE 时出错: " << strerror(-ret);
    if ((cqe->user_data & 1) == 1) {
      EXPECT_EQ(cqe->res, static_cast<int>(len));
      // LOG_INFO << "写入数据到: " << (cqe->user_data >> 1);
    } else {
      EXPECT_EQ(cqe->res, static_cast<int>(len));
      buffer[((cqe->user_data) >> 1) - 1][len] = '\0';
      // LOG_INFO << "from: " << (cqe->user_data >> 1)
      //  << ", 读取数据: " << buffer[((cqe->user_data) >> 1) - 1];
      EXPECT_STREQ(buffer[((cqe->user_data) >> 1) - 1], str);
    }
    io_uring_cqe_seen(&ring_, cqe);
  }
}

TEST_F(IoUringTest, MixWriteReadEventfdTest) {
  /**
   * @brief iouring
   * 是将IO完成事件乱序执行，所以应该由用户或者File来处理对同一文件的读写顺序
   *
   */
  // struct io_uring ring_;
  struct io_uring_params params;
  memset(&params, 0, sizeof(params));
  params.flags = 0;
  params.cq_entries = ENTRIES;
  params.sq_entries = BATCH_SIZE;
  // params.flags = 0; // 可选：使用内核轮询提升性能
  int ret = io_uring_queue_init_params(ENTRIES, &ring_, &params);

  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);
  // 注册 eventfd and epollfd
  int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  int epoll_fd = epoll_create1(0);
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = efd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, efd, &ev) < 0) {
    perror("epoll_ctl 失败");
    close(efd);
    io_uring_queue_exit(&ring_);
    return;
  }
  ret = io_uring_register_eventfd(&ring_, efd);
  ASSERT_GE(ret, 0) << "注册 eventfd 失败: " << strerror(-ret);

  int fd = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd, -1) << "文件打开失败";
  const char *str = "Hello, io_uring!";
  size_t len = strlen(str);
  // 提交 BATCH_SIZE 个写和读操作
  char buffer[BATCH_SIZE][128];
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe1 = io_uring_get_sqe(&ring_);
    // struct io_uring_sqe *sqe2 = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe1) << "无法获取 SQE";
    // ASSERT_TRUE(sqe2) << "无法获取 SQE";

    io_uring_prep_write(sqe1, fd, str, static_cast<unsigned int>(len),
                        static_cast<size_t>(i) * len);
    sqe1->user_data = static_cast<__U64_TYPE>((i + 1) << 1 | 1);
    ret = io_uring_submit(&ring_);
  }
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe2 = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe2) << "无法获取 SQE";

    io_uring_prep_read(sqe2, fd, buffer[i], sizeof(buffer[i]),
                       static_cast<size_t>(i) * len);
    sqe2->user_data = static_cast<__U64_TYPE>((i + 1) << 1 | 0);
    ret = io_uring_submit(&ring_);
  }

  // 提交所有 SQEs 到内核
  // LOG_INFO << "提交 SQEs: " << ret;
  sleep(1);

  // 主循环：非阻塞轮询 eventfd
  bool finished = false;
  while (!finished) {
    struct epoll_event events[1];
    int n = epoll_wait(epoll_fd, events, 1, 1000); // 非阻塞模式（timeout=0）
    if (n < 0) {
      perror("epoll_wait 失败");
      break;
    }

    if (n > 0) {
      // 7. 处理 eventfd 事件
      uint64_t event_count;
      ssize_t s = read(efd, &event_count, sizeof(event_count));
      if (s != sizeof(event_count)) {
        perror("读取 eventfd 失败");
        break;
      }

      // 8. 处理所有就绪的 CQE
      struct io_uring_cqe *cqe;
      unsigned count = 0;
      unsigned head;
      io_uring_for_each_cqe(&ring_, head, cqe) {
        if (cqe->res < 0) {
          fprintf(stderr, "I/O 错误: %s (user_data=%llu)\n",
                  strerror(-cqe->res), cqe->user_data);
        } else {
          // 检查用户数据是否正确，并验证读取的数据
          if ((cqe->user_data & 1) == 1) {
            EXPECT_EQ(cqe->res, static_cast<int>(len));
            // LOG_INFO << "写入数据到: " << (cqe->user_data >> 1);
          } else {
            // EXPECT_EQ(cqe->res, static_cast<int>(len));
            // buffer[((cqe->user_data) >> 1) - 1][len] = '\0';
            // LOG_INFO << "from: " << (cqe->user_data >> 1)
            //          << ", 读取数据: " << buffer[((cqe->user_data) >> 1) -
            //          1];
            // EXPECT_STREQ(buffer[((cqe->user_data) >> 1) - 1], str);
          }
        }
        count++;
      }
      io_uring_cq_advance(&ring_, count);
      EXPECT_EQ(count, 2 * BATCH_SIZE);
      finished = true;
    } else {
      // 9. 没有事件时执行其他任务（例如处理其他逻辑）
      printf("无事件，执行其他任务...\n");
      sleep(1); // 模拟其他工作
    }
  }
}

TEST_F(IoUringTest, MultiWriteEventfdTest) {
  int fd1 = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd1, -1) << "文件打开失败";
  int fd2 = open("test2.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd2, -1) << "文件打开失败";
  int fd3 = open("test3.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
  ASSERT_NE(fd3, -1) << "文件打开失败";

  const char *str1 = "Hello, io_uring1!";
  const char *str2 = "Hello, io_uring2!";
  const char *str3 = "Hello, io_uring3!";

  size_t len = strlen(str1);

  struct io_uring_params params;
  memset(&params, 0, sizeof(params));
  params.flags = 0;
  params.cq_entries = ENTRIES;
  params.sq_entries = BATCH_SIZE;
  // params.flags = 0; // 可选：使用内核轮询提升性能
  int ret = io_uring_queue_init_params(ENTRIES, &ring_, &params);

  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);
  // 注册 eventfd and epollfd
  int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  int epoll_fd = epoll_create1(0);
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = efd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, efd, &ev) < 0) {
    perror("epoll_ctl 失败");
    close(efd);
    io_uring_queue_exit(&ring_);
    return;
  }
  ret = io_uring_register_eventfd(&ring_, efd);
  ASSERT_GE(ret, 0) << "注册 eventfd 失败: " << strerror(-ret);

  // 提交 BATCH_SIZE 个写操作，分别向不同的文件写入数据
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe) << "无法获取 SQE";
    if (i % 3 == 0) {
      io_uring_prep_write(sqe, fd1, str1, static_cast<unsigned int>(len), 0);
      sqe->user_data =
          static_cast<__U64_TYPE>(((i + 1) << 4 & 0b0000) | 0b0111);
      // LOG_INFO << "Write in file 1";
    } else if (i % 3 == 1) {
      io_uring_prep_write(sqe, fd2, str2, static_cast<unsigned int>(len), 0);
      sqe->user_data =
          static_cast<__U64_TYPE>(((i + 1) << 4 & 0b0000) | 0b1011);
      // LOG_INFO << "Write in file 2";
    } else {
      io_uring_prep_write(sqe, fd3, str3, static_cast<unsigned int>(len), 0);
      sqe->user_data =
          static_cast<__U64_TYPE>(((i + 1) << 4 & 0b0000) | 0b1111);
      // LOG_INFO << "Write in file 3";
    }
    ret = io_uring_submit(&ring_);
  }

  // 提交所有 SQEs 到内核
  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);
  // EXPECT_EQ(ret, BATCH_SIZE);
  // 提交3个read request
  char buffer[3][128];
  for (int i = 0; i < 3; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    ASSERT_TRUE(sqe) << "无法获取 SQE";
    if (i == 0) {
      io_uring_prep_read(sqe, fd1, buffer[i], sizeof(buffer[i]), 0);
      sqe->user_data = static_cast<__U64_TYPE>(0);
    } else if (i == 1) {
      io_uring_prep_read(sqe, fd2, buffer[i], sizeof(buffer[i]), 0);
      sqe->user_data = static_cast<__U64_TYPE>(1);
    } else {
      io_uring_prep_read(sqe, fd3, buffer[i], sizeof(buffer[i]), 0);
      sqe->user_data = static_cast<__U64_TYPE>(2);
    }
    ret = io_uring_submit(&ring_);
  }
  // 提交所有 SQEs 到内核
  ASSERT_GE(ret, 0) << "提交 SQEs 失败: " << strerror(-ret);

  sleep(3);
  // eventfd 轮询
  bool finished = false;
  int count1{0}, count2{0}, count3{0};
  unsigned count = 0;
  while (!finished) {
    struct epoll_event events[1];
    int n = epoll_wait(epoll_fd, events, 1, 1000); // 非阻塞模式（timeout=0）
    if (n < 0) {
      perror("epoll_wait 失败");
      break;
    }

    if (n > 0) {
      // 7. 处理 eventfd 事件
      uint64_t event_count;
      ssize_t s = read(efd, &event_count, sizeof(event_count));
      if (s != sizeof(event_count)) {
        perror("读取 eventfd 失败");
        break;
      }

      // 8. 处理所有就绪的 CQE
      struct io_uring_cqe *cqe;
      unsigned head;
      io_uring_for_each_cqe(&ring_, head, cqe) {
        if (cqe->res < 0) {
          fprintf(stderr, "I/O 错误: %s (user_data=%llu)\n",
                  strerror(-cqe->res), cqe->user_data);
        } else {
          if ((cqe->user_data & 15) == 0) {
            EXPECT_EQ(cqe->res, static_cast<int>(len));
            buffer[0][len] = '\0';
            // LOG_INFO << "Read in file 1, data: " << buffer[0];
            EXPECT_STREQ(buffer[0], str1);
          } else if ((cqe->user_data & 15) == 1) {
            EXPECT_EQ(cqe->res, static_cast<int>(len));
            buffer[1][len] = '\0';
            // LOG_INFO << "Read in file 2, data: " << buffer[1];
            EXPECT_STREQ(buffer[1], str2);
          } else if ((cqe->user_data & 15) == 2) {
            EXPECT_EQ(cqe->res, static_cast<int>(len));
            buffer[2][len] = '\0';
            // LOG_INFO << "Read in file 3, data: " << buffer[2];
            EXPECT_STREQ(buffer[2], str3);
          } else {

            // 检查用户数据是否正确，并验证读取的数据
            if ((cqe->user_data & 15) == 7) {
              EXPECT_EQ(cqe->res, static_cast<int>(len));
              count1++;
              // LOG_INFO << "Write in file 1, count1 num: " << count1;
            } else if ((cqe->user_data & 15) == 11) {
              EXPECT_EQ(cqe->res, static_cast<int>(len));
              count2++;
              // LOG_INFO << "Write in file 2, count2 num: " << count2;
            } else {
              EXPECT_EQ(cqe->res, static_cast<int>(len));
              count3++;
              // LOG_INFO << "Write in file 3, count3 num: " << count3;
            }
          }
        }
        count++;
      }
      io_uring_cq_advance(&ring_, count);
      EXPECT_EQ(count, BATCH_SIZE + 3);
      // LOG_INFO << "count1: " << count1 << ", count2: " << count2
      //  << ", count3: " << count3;
      EXPECT_EQ(BATCH_SIZE, count1 + count2 + count3);
      finished = true;
    } else {
      // 没有事件时执行其他任务（例如处理其他逻辑）
      printf("无事件，执行其他任务...\n");
      sleep(1); // 模拟其他工作
    }
  }
}