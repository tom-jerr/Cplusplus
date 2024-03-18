#include <gtest/gtest.h>
#include <inttypes.h>
#include <sys/time.h>

#include "logger/CLFileUtil.hpp"
#include "logger/CLLogFile.hpp"
#include "logger/CLLogger.hpp"
#include "logger/CLThread.hpp"
#include "logger/CLTimeStamp.hpp"

TEST(TimeStampUnitTest, nowtest) {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  int64_t microseconds = tv.tv_usec;
  int64_t now = seconds * 1000 * 1000 + microseconds;

  TimeStamp t(now);
  int64_t now2 = t.microSecondSinceEpoch();
  EXPECT_EQ(now, now2);
}

TEST(TimeStampUnitTest, addtest) {
  TimeStamp t = TimeStamp::now();
  TimeStamp t2 = TimeStamp::addTime(t, 3.5);
  EXPECT_EQ(t2.microSecondSinceEpoch(),
            t.microSecondSinceEpoch() + 3.5 * 1000 * 1000);
  TimeStamp t3 = TimeStamp::addTime(t, 0.5);
  EXPECT_EQ(t3.microSecondSinceEpoch(),
            t.microSecondSinceEpoch() + 0.5 * 1000 * 1000);
}

TEST(TimeStampUnitTest, invalidtest) {
  TimeStamp t = TimeStamp::invalid();
  EXPECT_EQ(t.microSecondSinceEpoch(), 0);
  EXPECT_FALSE(t.valid());
}

TEST(TimeStampUnitTest, timeDifftest) {
  TimeStamp t = TimeStamp::now();
  TimeStamp t2 = TimeStamp::addTime(t, 3.5);
  double diff = TimeStamp::timeDifference(t2, t);
  EXPECT_EQ(diff, 3.5);
}

TEST(TimeStampUnitTest, toStringtest) {
  char buf[64] = {0};
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  int64_t microseconds = tv.tv_usec;
  snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds,
           microseconds);
  std::string str2 = buf;

  TimeStamp t(tv.tv_sec * 1000 * 1000 + tv.tv_usec);
  std::string str = t.toString();
  EXPECT_EQ(str, str2);
}

TEST(LogFileTest, threadsAppendtest) {
  const std::string str = "/home/lzy/Workspace/TinyNetLib/log/test.txt";
  LogFile file(str);
  Thread thread1(
      [&file]() { file.append("hello thread1\n", 14), file.flush(); });
  Thread thread2(
      [&file]() { file.append("hello thread2\n", 14), file.flush(); });
  thread1.start();
  thread2.start();
  file.append("hello main thread\n", 18);
  file.flush();
  thread1.join();
  thread2.join();
}

TEST(LoggerTest, loggertest) {
  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "Hello";
  LOG_WARN << "World";
  LOG_ERROR << "Error";
}