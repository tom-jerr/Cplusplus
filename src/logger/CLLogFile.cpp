#include "logger/CLLogFile.hpp"

#include <cassert>

#include "logger/CLFileUtil.hpp"

LogFile::LogFile(const std::string& filePath, off_t rollSize, bool threadSafe,
                 int flushInterval)
    : m_filepath_(filePath),
      m_flushInterval_(flushInterval),
      m_rollcnt_(0),
      m_rollSize_(rollSize),
      m_file_(new AppendFile(m_filepath_)),
      m_mutex_(threadSafe ? new std::mutex : nullptr) {
  assert(filePath.find('/') != std::string::npos);
  rollFile();
}

bool LogFile::rollFile() {
  // 初始化回滚文件
  if (static_cast<off_t>(m_file_->writtenBytes()) < m_rollSize_) {
    m_file_.reset(new AppendFile(m_filepath_));
  } else {
    // 已经写入的文件大小超过了rollSize，需要回滚文件
    assert(remove(m_filepath_.c_str()) == 0);
    m_file_.reset(new AppendFile(m_filepath_));
  }
  return true;
}

void LogFile::append(const char* logline, int len) {
  if (m_mutex_) {
    std::lock_guard<std::mutex> lock(*m_mutex_);
    append_unlocked(logline, len);
  } else {
    append_unlocked(logline, len);
  }
}

void LogFile::append_unlocked(const char* logline, int len) {
  m_file_->append(logline, len);
  if (static_cast<off_t>(m_file_->writtenBytes()) > m_rollSize_) {
    rollFile();
  }
}

void LogFile::flush() {
  if (m_mutex_) {
    std::lock_guard<std::mutex> lock(*m_mutex_);
    m_file_->flush();
  } else {
    m_file_->flush();
  }
}

std::string LogFile::getLogFileName(const std::string& basename) {
  std::string filename;
  filename.reserve(basename.size() + 32);
  filename = basename;

  char timebuf[24];
  struct tm tm;
  time_t now = time(nullptr);
  gmtime_r(&now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;
  filename += "log";
  return filename;
}