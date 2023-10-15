#include <fcntl.h>
#include <unistd.h>

#include "../include/CLLogUtils.h"
#include "../include/CLLogger.h"
namespace neo {
namespace log {
Logger::Logger() : logfd_(-1), wbuffer_(Buffer()), rbuffer_(Buffer()) {}
Logger::~Logger() {
  int ret;
  if (logfd_ != -1) {
    if ((ret = close(logfd_)) < 0) {
      printf("Error: ~Logger() failed\n");
    }
  }
}

/* file op*/

/* open filename */
int Logger::openFile(const char* filename) {
  logfd_ = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
  if (logfd_ == -1) {
    printf("Error: open file %s failed\n", filename);
    return -1;
  }
  return 1;
}

/* close filename*/
int Logger::closeFile(const char* filename) {
  int ret;
  if (logfd_ != -1) {
    if ((ret = close(logfd_)) < 0) {
      printf("Error: close file %s failed\n", filename);
      return -1;
    }
    logfd_ = -1;
  }
  return 1;
}

/* lseek file */
int Logger::lseekFile(const char* filename, int mode, int offset) {
  int ret;
  if (logfd_ != -1) {
    if ((ret = lseek(logfd_, offset, mode)) < 0) {
      printf("Error: lseek file %s failed\n", filename);
      return -1;
    }
  } else {
    printf("Error: no file %s opened, lseek failed\n", filename);
    return -1;
  }
  return 1;
}

/* flush buffer to file */
int Logger::flushBuffer() {
  int ret;
  if (logfd_ != -1) {
    /* 将整个buffer刷到磁盘上 */
    if ((ret = write(logfd_, wbuffer_.begin(),
                     BUFFER_SIZE - wbuffer_.avail())) < 0) {
      printf("Error: flush buffer to file failed\n");
      return -1;
    }
    /* 重新设置buffer */
    wbuffer_.reset();
  } else {
    printf("Error: no file opened, flush buffer failed\n");
    return -1;
  }
  return 1;
}

/* write log to buffer */
int Logger::writeLog(const char* logstr) {
  int len = strlen(logstr);
  /* if avail less than buffer avail, flush buffer and reset buffer */
  if (len > wbuffer_.avail()) {
    printf("Info: buffer is full, flush buffer\n");
    flushBuffer();
  }
  /* write logstr to buffer */
  wbuffer_.append(logstr, len);
  return 1;
}

/* Log op */

/* set the logfile you should write or read */
void Logger::setLogFile(const char* filename) {
  if (filename != nullptr) {
    if (logfd_ != -1) closeFile(filename);
    openFile(filename);
  }
}

}  // namespace log
}  // namespace neo