#ifndef _CLLOGGER_H
#define _CLLOGGER_H
#include <cstring>

#include "logger/CLLogStream.hpp"
#include "logger/CLTimeStamp.hpp"

#define LOG_TRACE                          \
  if (Logger::logLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG                          \
  if (Logger::logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO \
  if (Logger::logLevel() <= Logger::INFO) Logger(__FILE__, __LINE__).stream()
#define LOG_WARN                          \
  if (Logger::logLevel() <= Logger::WARN) \
  Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR                          \
  if (Logger::logLevel() <= Logger::ERROR) \
  Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL                          \
  if (Logger::logLevel() <= Logger::FATAL) \
  Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR                         \
  if (Logger::logLevel() <= Logger::ERROR) \
  Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL                       \
  if (Logger::logLevel() <= Logger::FATAL) \
  Logger(__FILE__, __LINE__, true).stream()

/**
 * @brief 日志类，使用桥接模式，具体实现由Impl实现
 */
class Logger {
 public:
  using outputFunc = void (*)(const char* msg, int len);
  using flushFunc = void (*)();
  // 日志级别
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };
  class SourceFile {
   public:
    template <int N>
    inline SourceFile(const char (&arr)[N]) : m_data_(arr), m_size_(N - 1) {
      const char* slash = strrchr(m_data_, '/');  // builtin function
      if (slash) {
        m_data_ = slash + 1;
        m_size_ -= static_cast<int>(m_data_ - arr);
      }
    }
    explicit SourceFile(const char* filename) : m_data_(filename) {
      const char* slash = strrchr(filename, '/');
      if (slash) {
        m_data_ = slash + 1;
      }
      m_size_ = static_cast<int>(strlen(m_data_));
    }
    const char* m_data_;
    int m_size_;
  };
  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  static void setLogLevel(LogLevel level);
  static LogLevel logLevel();

  LogStream& stream() { return m_impl_.m_stream_; }

  static void setOutput(outputFunc);
  static void setFlush(flushFunc);

 private:
  class Impl {
   public:
    Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
    void formatTime();
    void finish();
    TimeStamp m_time_;
    LogStream m_stream_;
    LogLevel m_level_;
    int m_line_;
    SourceFile m_basename_;
  };
  Impl m_impl_;
};

const char* strerror_tl(int savedErrno);
#endif /* _CLLOGGER_H*/