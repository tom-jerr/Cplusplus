#ifndef CLLOG_H_
#define CLLOG_H_
#include <string>

class Log {
  private:
    Log();
  private:
    std::string m_logFileName_;
  public:
    static Log* m_logInstance;
    static Log* getLogInstance() {
      if (m_logInstance == nullptr) {
        m_logInstance = new Log();
      }
      return m_logInstance;
    }
    void writeLog(const char* logMsg);
};

Log* Log::m_logInstance = nullptr;
Log::Log() {
  m_logFileName_ = "../../Log/DenoiseLog.txt";
}

void Log::writeLog(const char* logMsg) {
  FILE* fp = fopen(m_logFileName_.c_str(), "a+");
  if (fp == nullptr) {
    return;
  }
  fprintf(fp, "%s\n", logMsg);
  fclose(fp);
}

#endif /* CLLOG_H_ */