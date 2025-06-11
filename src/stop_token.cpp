#include <chrono>
#include <iostream>
#include <stop_token>

int main() {
    std::stop_source source;
    std::stop_token token = source.get_token();

    // 模拟一些可以被取消的工作
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::seconds(10); // 设定10秒后结束任务

    while (std::chrono::steady_clock::now() < endTime) {
        if (token.stop_requested()) {
            std::cout << "Task was canceled!" << std::endl;
            break;
        }
        std::cout << "Working..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 模拟在某个条件下请求停止
        if (std::chrono::steady_clock::now() >
            startTime + std::chrono::seconds(5)) {
            source.request_stop();
        }
    }

    if (!token.stop_requested()) {
        std::cout << "Task completed normally." << std::endl;
    }

    return 0;
}