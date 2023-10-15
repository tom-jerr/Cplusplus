#ifndef CLLOGUTILS_H_
#define CLLOGUTILS_H_
#define BUFFER_SIZE 2048
#include <string.h>

#include <cstdlib>
#include <string>
namespace neo {
namespace log {

/* 日志缓存类，可容纳2048B*/
class Buffer {
 public:
  Buffer();
  ~Buffer();
  /* add log to buffer */
  int append(const char* str, int len);
  /* return the begin of buffer */
  const char* begin() const { return data_; }
  /* return the current pos of buffer */
  const char* cur() const { return cur_; }
  /* return the end of buffer*/
  const char* end() const { return data_ + BUFFER_SIZE; }
  /* return avail length of buffer*/
  int avail() const { return avail_; }
  /* reset buffer */
  void reset();

 private:
  /* current pos of buffer */
  char* cur_;
  /* data in the buffer */
  char* data_;
  /* available size in buffer */
  int avail_;
};

}  // namespace log
}  // namespace neo

#endif  // CLLOGUTILS_H_