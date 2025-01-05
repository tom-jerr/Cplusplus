#pragma once
#include <cstdint>

class Test{
  public:
    Test();
    Test(int value) : data_(value){}
    ~Test();

    void setData(int value);
    int getData() const;
  private:
    void set(int value);
    int get() const;
  private:
    int data_;


};