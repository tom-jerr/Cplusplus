#ifndef CLLOGGER_H_
#define CLLOGGER_H_
#include "CLLogUtils.h"

namespace neo {
namespace log {

class Logger {
 public:
  /* log level*/
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  /* lseek mode */
  enum Mode {
    BEGIN,
    CURRENT,
    END,
  };

  Logger();
  ~Logger();

  /* log op */

  /* set log file name */
  void setLogFile(const char* filename);
  /* write log */
  int writeLog(const char* logstr);
  /* read log */
  int readLog(const char* filename, int len);

  /* file op */

  /* open filename */
  int openFile(const char* filename);
  /* close filename*/
  int closeFile(const char* filename);
  /* return offset in filename under mode */
  int lseekFile(const char* filename, int mode, int offset);

 private:
  /* file op */
  /* read len data from filename*/
  int readFile(const char* filename, int len);
  /* write log to buffer */
  int writeFile(const char* str, int len);
  /* flush buffer to file */
  int flushBuffer();

 private:
  /* store log data to write */
  Buffer wbuffer_;
  /* store data read from file */
  Buffer rbuffer_;
  /* file descriptor*/
  int logfd_;
};
}  // namespace log
}  // namespace neo
#endif  // CLLOGGER_H_