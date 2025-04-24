/**
 * @file RequestContext.h
 * @author lzy
 * @brief
 * @version 0.1
 * @date 2025-04-23
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"
#include <future>
#include <memory>

namespace muduo {
namespace net {

class RequestContext {
public:
  RequestContext(Channel *channel, void *buffer, size_t data_size,
                 size_t offset, size_t req_id,
                 RequestType type = RequestType::None,
                 std::shared_ptr<std::promise<bool>> promise_ptr = nullptr)
      : channel_(channel), buffer_(buffer), data_size_(data_size),
        offset_(offset), id_(req_id), type_(type), promise_ptr_(promise_ptr) {}
  ~RequestContext() = default;
  Channel *getChannel() const { return channel_; }
  void *getBuffer() const { return buffer_; }
  size_t getDataSize() const { return data_size_; }
  size_t getOffset() const { return offset_; }
  size_t getReqId() const { return id_; }
  RequestType getRequestType() const { return type_; }
  void setComplete() { promise_ptr_->set_value(true); }

private:
  Channel *channel_;
  void *buffer_;
  size_t data_size_{0};
  size_t offset_{0};
  size_t id_;
  RequestType type_{RequestType::None};
  std::shared_ptr<std::promise<bool>> promise_ptr_;
};
} // namespace net
} // namespace muduo