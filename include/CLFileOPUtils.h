#ifndef CLfileUTILS_H_
#define CLfileUTILS_H_
#define BUFFER_SIZE 2048

namespace neo {
namespace file {

/* 文件操作缓存类，可容纳2048B*/
class Buffer {
 public:
  Buffer();
  virtual ~Buffer();
  /* add file to buffer */
  int append(const char* str, int len);
  /* return the begin of buffer */
  const char* begin() const { return data_; }
  /* return the current pos of buffer */
  const char* cur() const { return cur_; }
  /* return the end of buffer*/
  virtual const char* end() const { return data_ + BUFFER_SIZE; }
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

/* header metadata */
struct Header {
  int file_off;  // 文件中的偏移
  int data_len;  // 数据长度
};

/* read header buffer for data from disk */
class HBuffer {
 public:
  HBuffer();
  ~HBuffer();
  /* append header into HBuffer */
  int append(const Header* header);
  void reset();

  /* get member op */
  Header* getHeader() { return header_; }
  int getCount() const { return count_; }

 private:
  Header* header_;
  int avail_;  // 可用header数量
  int count_;  // 当前header数量
};
}  // namespace file
}  // namespace neo

#endif  // CLfileUTILS_H_