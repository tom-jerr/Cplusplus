#include <gtest/gtest.h>
#include <inttypes.h>
#include <stdio.h>

#include <sstream>

#include "../../include/logger/CLLogStream.hpp"
#include "../../include/logger/CLTimeStamp.hpp"

const size_t N = 1000000;

template <typename T>
void benchPrintf(const char* fmt) {
  char buf[32];
  TimeStamp start(TimeStamp::now());
  for (size_t i = 0; i < N; ++i) snprintf(buf, sizeof buf, fmt, (T)(i));
  TimeStamp end(TimeStamp::now());

  printf("benchPrintf %f\n", TimeStamp::timeDifference(end, start));
}

template <typename T>
void benchStringStream() {
  TimeStamp start(TimeStamp::now());
  std::ostringstream os;

  for (size_t i = 0; i < N; ++i) {
    os << (T)(i);
    os.seekp(0, std::ios_base::beg);
  }
  TimeStamp end(TimeStamp::now());

  printf("benchStringStream %f\n", TimeStamp::timeDifference(end, start));
}

template <typename T>
void benchLogStream() {
  TimeStamp start(TimeStamp::now());
  LogStream os;
  for (size_t i = 0; i < N; ++i) {
    os << (T)(i);
    // os.resetBuffer();
  }
  TimeStamp end(TimeStamp::now());

  printf("benchLogStream %f\n", TimeStamp::timeDifference(end, start));
}

TEST(BenchmarkTest, LogStreamtest) {
  puts("------------------------ int ------------------------");
  benchPrintf<int>("%d");
  benchStringStream<int>();
  benchLogStream<int>();

  puts("------------------------ double  ------------------------");
  benchPrintf<double>("%.12g");
  benchStringStream<double>();
  benchLogStream<double>();

  puts("------------------------ int64_t ------------------------");
  benchPrintf<int64_t>("%" PRId64);
  benchStringStream<int64_t>();
  benchLogStream<int64_t>();

  puts("------------------------ void* ------------------------");
  benchPrintf<void*>("%p");
  benchStringStream<void*>();
  benchLogStream<void*>();
}