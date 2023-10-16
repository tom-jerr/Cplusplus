#include "../include/CLFileOPUtils.h"

#include <string.h>

#include <cstdio>
namespace neo {
namespace file {
/* Buffer */

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

/* add file to buffer */
/* if len larger than buffer, CLfileger can write buffer to file and reset
 * buffer
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

/* HBuffer */

HBuffer::HBuffer()
    : header_(new Header[BUFFER_SIZE]), avail_(BUFFER_SIZE), count_(0) {}

HBuffer::~HBuffer() {
  reset();
  delete[] header_;
}

/* store header to HBuffer */
int HBuffer::append(const Header* header) {
  if (avail_ > 0) {
    header_[count_++] = *header;
    avail_--;
    return 1;
  }
  printf("Info: buffer is full\n");
  return 0;
}

/* reset HBuffer, just overlap this buffer again */
void HBuffer::reset() {
  avail_ = BUFFER_SIZE;
  count_ = 0;
}
}  // namespace file
}  // namespace neo