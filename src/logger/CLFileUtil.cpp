#include "logger/CLFileUtil.hpp"

#include <sys/stat.h>
#include <sys/types.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

/**
 * @brief 获取文件大小
 * @note 通过文件路径获取文件大小，使用Linux下的stat函数
 * @param path 文件路径
 * @return 文件大小
 */
static off_t fileSize(const std::string& path);
/**
  * @brief 构造函数
  * @note 打开文件，设置文件缓冲区；
  * @param filename 文件名

*/
AppendFile::AppendFile(const std::string& filename)
    : m_fp_(::fopen(filename.c_str(), "ae")),
      m_writtenBytes_(fileSize(filename.c_str())) {
  assert(m_fp_);
  ::setbuffer(m_fp_, m_buffer_, sizeof m_buffer_);
  // write((filename + "\n").c_str(), strlen(filename.c_str()));
}

AppendFile::~AppendFile() { ::fclose(m_fp_); }

void AppendFile::append(const char* logline, const size_t len) {
  size_t nread = write(logline, len);
  size_t remain = len - nread;
  while (remain > 0) {
    size_t x = write(logline + nread, remain);
    if (x == 0) {
      int err = ferror(m_fp_);
      if (err) {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
      }
      break;
    }
    nread += x;
    remain = len - nread;
  }

  m_writtenBytes_ += len;
}

size_t AppendFile::write(const char* logline, const size_t len) {
  return ::fwrite_unlocked(logline, 1, len, m_fp_);
}

void AppendFile::flush() { ::fflush(m_fp_); }

size_t AppendFile::readFromBuffer(char* buffer, size_t len) {
  size_t nread = ::fread_unlocked(buffer, 1, len, m_fp_);
  return nread;
}

static off_t fileSize(const std::string& path) {
  struct stat statbuf;
  if (::stat(path.c_str(), &statbuf) < 0) {
    return 0;
  } else {
    return statbuf.st_size;
  }
}