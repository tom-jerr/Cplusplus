#ifndef _CLLOGFILE_H
#define _CLLOGFILE_H

#include <sys/types.h>

#include <memory>
#include <mutex>

#include "logger/CLFileUtil.hpp"
class AppendFile;
/**
 * @brief 日志文件类
 * @note
 * 日志文件类，用于写日志文件，提供对日志文件的操作，包括滚动日志文件、将log数据写到当前log文件、flush
 * log数据到当前log文件
 */
class LogFile {
 public:
  LogFile(const std::string& filePath, off_t rollSize = 2048 * 1000,
          bool threadSafe = true, int flushInterval = 0);
  ~LogFile() = default;

  void append(const char* logline, int len);
  void flush();
  std::string getLogFileName(const std::string& basename);

 private:
  void append_unlocked(const char* logline, int len);
  bool rollFile();

  const std::string m_filepath_;
  const int m_flushInterval_;

  int m_rollcnt_;
  off_t m_rollSize_;
  std::unique_ptr<AppendFile> m_file_;
  std::unique_ptr<std::mutex> m_mutex_;
};
#endif /* _CLLOGFILE_H */