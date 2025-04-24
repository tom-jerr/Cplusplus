/**
 * @file FileUring.h
 * @author lzy
 * @brief
 * @version 0.1
 * @date 2025-04-23
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef MUDUO_NET_IOURINGFILE_FILEURING_H
#define MUDUO_NET_IOURINGFILE_FILEURING_H

#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThreadPool.h"
#include <cstddef>
#include <fcntl.h>
// #include <unordered_map>
// #include <map>
#include <future>
#include <memory>
namespace muduo {
namespace net {

/**
 * @brief This class if for async file operation by iouring, we don't want
 * combine it with EventLoopThreadPool, because we maybe have many files, and we
 * don't want to create many EventLoopThreadPool, so we just use one
 * EventLoopThreadPool, we just first define one EventLoopThreadPool, and then
 * create many FileUring objects, and then submit IO operations to
 * EventLoopThreadPool, and execute eventloop callback and user callback.
 *
 */
class FileUring {

public:
  FileUring(const std::string &file_path, EventLoop *loop, bool whether_direct);

  ~FileUring();

  std::future<bool> asyncRead(void *buffer, size_t data_size, size_t offset,
                              std::function<void()> callback);

  std::future<bool> asyncWrite(void *str, size_t data_size, size_t offset,
                               std::function<void()> callback);

private:
  Channel *file_channel_;
  std::string file_path_;
  bool whether_direct_{false};
  size_t request_id_{0};
};

} // namespace net

} // namespace muduo

#endif // MUDUO_NET_IOURINGFILE_FILEURING_H