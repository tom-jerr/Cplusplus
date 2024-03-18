#ifndef _CLLOGSTREAM_H
#define _CLLOGSTREAM_H

#include <cstddef>
#include <cstring>
#include <string>

const int kSmallBuffer = 4096;
const int kLargeBuffer = 4096 * 1000;

template <int SIZE>
class LogBuffer {
 public:
  LogBuffer() : m_cur_(m_data_) {}
  ~LogBuffer() = default;

  void append(const char* buf, size_t len) {
    if (avail() > static_cast<int>(len)) {
      memcpy(m_cur_, buf, len);
      m_cur_ += len;
    }
  }

  const char* end() const { return m_data_ + sizeof(m_data_); }

  const char* data() const { return m_data_; }
  int length() const { return static_cast<int>(m_cur_ - m_data_); }
  char* current() { return m_cur_; }

  int avail() const { return static_cast<int>(end() - m_cur_); }
  void add(size_t len) { m_cur_ += len; }

  void reset() { m_cur_ = m_data_; }
  /**
   * @brief 清空缓冲区
   */
  void bzero() { memset(m_data_, 0, sizeof(m_data_)); }

 private:
  char m_data_[SIZE];
  char* m_cur_;
};

/**
 * @brief 日志流类
 * @note 用于日志的输入输出(性能)，并非线程安全，每个消息有一个LogStream对象
 */
class LogStream {
 public:
  using Buffer = LogBuffer<kSmallBuffer>;
  using self = LogStream;

  LogStream() = default;
  ~LogStream() = default;

  self& operator<<(bool v);
  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float v);
  self& operator<<(double);

  self& operator<<(char v);
  self& operator<<(const char*);

  self& operator<<(const std::string& s);

  void append(const char* data, int len) { return m_buffer_.append(data, len); }
  const Buffer& buffer() const { return m_buffer_; }
  void resetBuffer() { m_buffer_.reset(); }

 private:
  /**
   * @brief 格式化整数
   * @tparam T 整数类型
   * @param value 待格式化的整数
   */
  template <typename T>
  void formatInteger(T);

  Buffer m_buffer_;
  static const int kMaxNumericSize = 32;
};

/**
 * @brief 格式化类
 * @note 用于格式化输出，数值类型数据转化成一个长度不超过32位的字符串对象
 */
class Fmt {
 public:
  template <typename T>
  Fmt(const char* fmt, T val);
  const char* data() const { return m_data_; }
  int length() const { return m_length_; }

 private:
  char m_data_[32];
  int m_length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
  s.append(fmt.data(), fmt.length());
  return s;
}

#endif /* _CLLOGSTREAM_H */