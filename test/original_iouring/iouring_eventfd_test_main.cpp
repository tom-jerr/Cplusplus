/**
 * @file iouring_eventfd_test.cpp
 * @author lzy
 * @brief
 * @version 0.1
 * @date 2025-04-22
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <cstring>
#include <fcntl.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#define ENTRIES 8
#define EVENTFD_FLAG 0
#define BATCH_SIZE 16 // 批量处理的任务数量
int main() {
  struct io_uring ring;
  int ret;

  // 2. 初始化 io_uring，启用事件通知
  struct io_uring_params params;
  memset(&params, 0, sizeof(params));
  params.flags = 0;
  params.cq_entries = ENTRIES;
  params.sq_entries = BATCH_SIZE;
  // params.flags = 0; // 可选：使用内核轮询提升性能
  ret = io_uring_queue_init_params(ENTRIES, &ring, &params);
  if (ret < 0) {
    fprintf(stderr, "io_uring 初始化失败: %s\n", strerror(-ret));

    return 1;
  }
  // initialize the eventfd, notify the io_uring
  int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (efd < 0) {
    perror("eventfd 创建失败");
    return 1;
  }
  // 3. 将 eventfd 注册到 io_uring 完成事件通知
  ret = io_uring_register_eventfd(&ring, efd);
  if (ret < 0) {
    fprintf(stderr, "注册 eventfd 失败: %s\n", strerror(-ret));
    close(efd);
    io_uring_queue_exit(&ring);
    return 1;
  }

  // 4. 初始化 epoll 监听 eventfd
  int epoll_fd = epoll_create1(0);
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = efd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, efd, &ev) < 0) {
    perror("epoll_ctl 失败");
    close(efd);
    io_uring_queue_exit(&ring);
    return 1;
  }

  // 5. 提交示例 I/O 任务（这里以写入文件为例）
  int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  const char *str = "Event-driven io_uring!";
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
    io_uring_submit(&ring);
  }

  sleep(1); // 测试是否可以按批提交

  // 6. 主循环：非阻塞轮询 eventfd
  while (1) {
    struct epoll_event events[1];
    int n = epoll_wait(epoll_fd, events, 1, 0); // 非阻塞模式（timeout=0）
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
      io_uring_for_each_cqe(&ring, head, cqe) {
        if (cqe->res < 0) {
          fprintf(stderr, "I/O 错误: %s (user_data=%llu)\n",
                  strerror(-cqe->res), cqe->user_data);
        } else {
          printf("操作成功: user_data=%llu, 结果=%d\n", cqe->user_data,
                 cqe->res);
        }
        count++;
      }
      io_uring_cq_advance(&ring, count);
    } else {
      // 9. 没有事件时执行其他任务（例如处理其他逻辑）
      printf("无事件，执行其他任务...\n");
      sleep(1); // 模拟其他工作
    }
  }

  // 清理资源
  close(fd);
  close(efd);
  close(epoll_fd);
  io_uring_queue_exit(&ring);
  return 0;
}