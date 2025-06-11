#include <barrier>
#include <cassert>
#include <iostream>
#include <thread>

class A {
public:
    void f() {
        std::barrier sync_point{3, [&]() noexcept { ++i_; }};
        for (auto &x : tasks_) {
            x = std::thread([&] {
                std::cout << 1;
                sync_point.arrive_and_wait();
                assert(i_ == 1);
                std::cout << 2;
                sync_point.arrive_and_wait();
                assert(i_ == 2);
                std::cout << 3;
            });
        }
        for (auto &x : tasks_) {
            x.join(); // 析构 barrier 前 join 所有使用了 barrier 的线程
        } // 析构 barrier 时，线程再调用 barrier 的成员函数是 undefined behavior
    }

private:
    std::thread tasks_[3] = {};
    int i_ = 0;
};

int main() {
    A a;
    a.f();
}