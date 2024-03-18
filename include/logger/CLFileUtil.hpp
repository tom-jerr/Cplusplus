#ifndef _CLFILEUTIL_H
#define _CLFILEUTIL_H
#include <cstddef>
#include <string>

class AppendFile {
 public:
  explicit AppendFile(const std::string& filename);
  ~AppendFile();

  void append(const char* logline, const size_t len);
  void flush();
  size_t readFromBuffer(char* buffer, size_t len);

  size_t writtenBytes() const { return m_writtenBytes_; }

 private:
  static const size_t kFileBufferSize = 4096;
  size_t write(const char* logline, size_t len);

  FILE* m_fp_;
  char m_buffer_[64 * 1024];
  size_t m_writtenBytes_;
};
#endif /* _CLFILEUTIL_H */