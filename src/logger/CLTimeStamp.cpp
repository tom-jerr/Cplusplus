#include "logger/CLTimeStamp.hpp"

#include <inttypes.h>
#include <sys/time.h>
TimeStamp::TimeStamp(int64_t microSecondSinceEpoch)
    : m_microSecondSinceEpoch_(microSecondSinceEpoch) {}

std::string TimeStamp::toString() const {
  char buf[64] = {0};
  int64_t seconds = m_microSecondSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = m_microSecondSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds,
           microseconds);
  return buf;
}

TimeStamp TimeStamp::now() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  return TimeStamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

TimeStamp TimeStamp::invalid() { return TimeStamp(); }

double TimeStamp::timeDifference(const TimeStamp &high, const TimeStamp &low) {
  int64_t diff = high.microSecondSinceEpoch() - low.microSecondSinceEpoch();
  return static_cast<double>(diff) / kMicroSecondsPerSecond;
}

TimeStamp TimeStamp::addTime(TimeStamp timestamp, double seconds) {
  int64_t delta =
      static_cast<int64_t>(seconds * timestamp.kMicroSecondsPerSecond);
  return TimeStamp(timestamp.microSecondSinceEpoch() + delta);
}