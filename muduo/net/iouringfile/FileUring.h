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
#include <cstddef>
#include <fcntl.h>
#include <unordered_map>
// #include <map>

namespace muduo {
namespace net {
class FileUring {

public:
  FileUring(const std::string &file_path, EventLoop *loop, bool whether_direct);

  ~FileUring();

  void asyncRead(void *buffer, size_t data_size, size_t offset,
                 std::function<void()> callback);

  void asyncWrite(void *str, size_t data_size, size_t offset,
                  std::function<void()> callback);

private:
  Channel *file_channel_;
  std::string file_path_;
  bool whether_direct_{false};
  size_t request_id_{0};
  std::unordered_map<size_t, RequestContext *> request_map_;
};

} // namespace net

} // namespace muduo

#endif // MUDUO_NET_IOURINGFILE_FILEURING_H