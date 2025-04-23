/**
 * @file iouring_test.cpp
 * @author lzy
 * @brief
 * @version 0.1
 * @date 2025-04-22
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define ENTRIES 1024  // io_uring 队列大小
#define BATCH_SIZE 16 // 批量处理的任务数量

int main() {
  struct io_uring ring;
  int ret;

  // 初始化 io_uring 实例
  ret = io_uring_queue_init(ENTRIES, &ring, 0);
  if (ret < 0) {
    fprintf(stderr, "io_uring 初始化失败: %s\n", strerror(-ret));
    return 1;
  }

  // 打开测试文件
  int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    perror("文件打开失败");
    return 1;
  }

  const char *str = "Hello, io_uring!";
  size_t len = strlen(str);

  // 提交 BATCH_SIZE 个写操作
  for (int i = 0; i < BATCH_SIZE; ++i) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
      fprintf(stderr, "无法获取 SQE\n");
      return 1;
    }
    io_uring_prep_write(sqe, fd, str, static_cast<unsigned int>(len),
                        0); // 准备写请求
    sqe->user_data =
        static_cast<__U64_TYPE>(i + 1); // 设置用户标识（避免使用 0）
  }

  // 提交所有 SQEs 到内核
  ret = io_uring_submit(&ring);
  if (ret < 0) {
    fprintf(stderr, "提交 SQEs 失败: %s\n", strerror(-ret));
    return 1;
  }

  // 批量处理 CQE
  int remaining = BATCH_SIZE;
  while (remaining > 0) {
    struct io_uring_cqe *cqe;
    // 等待至少一个 CQE 就绪
    ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
      fprintf(stderr, "等待 CQE 失败: %s\n", strerror(-ret));
      break;
    }

    // 遍历当前所有可用 CQE
    unsigned head;
    unsigned count = 0;
    io_uring_for_each_cqe(&ring, head, cqe) {
      if (cqe->res < 0) {
        // 处理错误
        fprintf(stderr, "写操作错误: %s (user_data=%lu)\n", strerror(-cqe->res),
                static_cast<unsigned long>(cqe->user_data));
      } else {
        // 处理成功结果
        printf("写操作成功: user_data=%lu, 写入字节=%d\n",
               static_cast<unsigned long>(cqe->user_data), cqe->res);
      }
      count++;
      remaining--;
    }

    // 推进 CQ 环，释放处理过的 CQE
    io_uring_cq_advance(&ring, count);
  }

  // 清理资源
  close(fd);
  io_uring_queue_exit(&ring);

  return 0;
}