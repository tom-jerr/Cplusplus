#include "class.h"

Test::Test() : data_(0) {}
Test::~Test() = default;

int Test::get() const {
  return data_;
}

void Test::set(int value) {
  data_ = value;
}

void Test::setData(int value) {
  set(value);
}

int Test::getData() const {
  return get();
}
