#ifndef _CLTIMESTAMP_H
#define _CLTIMESTAMP_H

#include <cstdint>
#include <string>
class TimeStamp {
 public:
  explicit TimeStamp(int64_t microSecondSinceEpoch);
  TimeStamp() : m_microSecondSinceEpoch_(0) {}
  ~TimeStamp() = default;

  bool operator<(const TimeStamp& rhs) const {
    return m_microSecondSinceEpoch_ < rhs.m_microSecondSinceEpoch_;
  }

  std::string toString() const;
  int64_t microSecondSinceEpoch() const { return m_microSecondSinceEpoch_; }
  static TimeStamp addTime(TimeStamp timestamp, double seconds);
  static double timeDifference(const TimeStamp& high, const TimeStamp& low);
  static TimeStamp now();
  /**
   * @brief 返回一个无效的时间戳s
   * 此时还没有Timestamp对象，也就无法通过对象的成员函数来构造自身对象，需要设置为static。
   *
   * @return TimeStamp
   */
  static TimeStamp invalid();
  bool valid() const { return m_microSecondSinceEpoch_ > 0; }

  void swap(TimeStamp& that) {
    std::swap(m_microSecondSinceEpoch_, that.m_microSecondSinceEpoch_);
  }
  static const int kMicroSecondsPerSecond = 1000 * 1000;
  static const int kMicroSecondsPerMillisecond = 1000;

 private:
  // 记录从epoch时间到现在的微秒数
  int64_t m_microSecondSinceEpoch_;
};

#endif /* _CLTIMESTAMP_H */