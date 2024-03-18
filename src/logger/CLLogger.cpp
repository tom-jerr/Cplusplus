#include "logger/CLLogger.hpp"

#include <time.h>

#include <cassert>

#include "logger/CLLogStream.hpp"

/* 线程局部变量 */
__thread char t_time[64];
__thread time_t t_lastSecond;
__thread char t_errnobuf[512];

static const char black[] = {0x1b, '[', '1', ';', '3', '0', 'm', 0};
static const char red[] = {0x1b, '[', '1', ';', '3', '1', 'm', 0};
static const char green[] = {0x1b, '[', '1', ';', '3', '2', 'm', 0};
static const char yellow[] = {0x1b, '[', '1', ';', '3', '3', 'm', 0};
static const char blue[] = {0x1b, '[', '1', ';', '3', '4', 'm', 0};
static const char purple[] = {0x1b, '[', '1', ';', '3', '5', 'm', 0};
static const char normal[] = {0x1b, '[', '0', ';', '3', '9', 'm', 0};

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "[TRACE]", "[DEBUG]", "[INFO ]", "[WARN ]", "[ERROR]", "[FATAL]",
};

const char* strerror_tl(int savedErrno) {
  return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

void defaultOutput(const char* msg, int len) {
  size_t n = fwrite(msg, 1, len, stdout);
  (void)n;
}

void defaultFlush() { fflush(stdout); }

Logger::outputFunc g_output = defaultOutput;
Logger::flushFunc g_flush = defaultFlush;
Logger::LogLevel g_logLevel = Logger::TRACE;

void Logger::setLogLevel(LogLevel level) { g_logLevel = level; }

Logger::LogLevel Logger::logLevel() { return g_logLevel; }

// helper class for known string length at compile time
class T {
 public:
  T(const char* str, unsigned len) : m_str(str), m_len(len) {
    assert(strlen(str) == m_len);
  }

  const char* m_str;
  const unsigned m_len;
};

Logger::Logger(SourceFile file, int line) : m_impl_(INFO, 0, file, line) {}
Logger::Logger(SourceFile file, int line, LogLevel level)
    : m_impl_(level, 0, file, line) {}
Logger::Logger(SourceFile file, int line, bool toAbort)
    : m_impl_(toAbort ? FATAL : ERROR, errno, file, line) {}
Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : m_impl_(level, 0, file, line) {
  m_impl_.m_stream_ << '[' << func << ']';
}

Logger::~Logger() {
  m_impl_.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (m_impl_.m_level_ == FATAL) {
    g_flush();
    abort();
  }
}

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file,
                   int line)
    : m_time_(TimeStamp::now()),
      m_stream_(),
      m_level_(level),
      m_line_(line),
      m_basename_(file) {
  formatTime();

  switch (level) {
    case TRACE:
      m_stream_ << green << LogLevelName[level] << normal << ' ';
      break;
    case DEBUG:
      m_stream_ << blue << LogLevelName[level] << normal << ' ';
      break;
    case INFO:
      m_stream_ << black << LogLevelName[level] << normal << ' ';
      break;
    case WARN:
      m_stream_ << yellow << LogLevelName[level] << normal << ' ';
      break;
    case ERROR:
      m_stream_ << purple << LogLevelName[level] << normal << ' ';
      break;
    case FATAL:
      m_stream_ << red << LogLevelName[level] << normal << ' ';
      break;
    default:
      m_stream_ << LogLevelName[level] << ' ';
      break;
  }

  // m_stream << LogLevelName[level] << ' ';
  m_stream_ << '[' << m_basename_.m_data_ << ':' << m_line_ << "] ";
  if (savedErrno != 0) {
    m_stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
  }
}

void Logger::Impl::finish() { m_stream_ << '\n'; }

void Logger::Impl::formatTime() {
  int64_t microSecondsSinceEpoch = m_time_.microSecondSinceEpoch();
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch /
                                       TimeStamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(microSecondsSinceEpoch %
                                      TimeStamp::kMicroSecondsPerSecond);
  if (seconds != t_lastSecond) {
    t_lastSecond = seconds;
    struct tm tm_time;

    ::gmtime_r(&seconds, &tm_time);  // FIXME TimeZone::fromUtcTime

    int len =
        snprintf(t_time, sizeof(t_time), "%4d-%02d-%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour + 8, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 19);
    (void)len;
  }

  Fmt us(".%06d ", microseconds);
  assert(us.length() == 8);
  m_stream_ << t_time << us.data();
}