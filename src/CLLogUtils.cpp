#include "../include/CLLogUtils.h"

#include <cstdio>

namespace neo {
namespace log {
/* malloc space for data */
Buffer::Buffer() : avail_(BUFFER_SIZE) {
  data_ = new char[BUFFER_SIZE];
  cur_ = data_;
}

/* free data's space*/
Buffer::~Buffer() {
  cur_ = nullptr;
  avail_ = 0;
  delete[] data_;
}

/* add log to buffer */
/* if len larger than buffer, CLLogger can write buffer to file and reset buffer
 */
int Buffer::append(const char* str, int len) {
  if (avail_ > len) {
    memcpy(cur_, str, len);
    cur_ += len;
    avail_ -= len;
    return 1;
  }
  printf("Info: buffer is full\n");
  return 0;
}

/* reset the buffer */
void Buffer::reset() {
  cur_ = data_;
  avail_ = BUFFER_SIZE;
}
}  // namespace log
}  // namespace neo