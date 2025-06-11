#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// 一个可以被中断的等待工具
struct InterruptibleWait {
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop_flag{false};

    void interrupt() {
        stop_flag.store(true);
        cv.notify_all(); // 唤醒所有等待的线程
    }

    // 等待，直到被通知或被中断
    template <typename Predicate>
    void wait(std::unique_lock<std::mutex> &lock, Predicate pred) {
        cv.wait(lock, [&] {
            return stop_flag.load() || pred(); // 等待条件满足 或 中断信号为真
        });
    }
};

// 示例：一个从队列中取任务的工作者
void queue_worker(InterruptibleWait &waiter, std::vector<int> &queue) {
    while (!waiter.stop_flag.load()) {
        std::unique_lock<std::mutex> lock(waiter.mtx);

        // 等待队列不为空，或者收到了中断信号
        waiter.wait(lock, [&] { return !queue.empty(); });

        // 从 wait 返回后，必须再次检查中断标志，以区分是条件满足还是被中断
        if (waiter.stop_flag.load()) {
            std::cout << "[Queue Worker] Interrupted while waiting. Exiting."
                      << std::endl;
            break;
        }

        // 如果代码能执行到这里，说明是队列不为空导致的唤醒
        int task = queue.back();
        queue.pop_back();
        lock.unlock(); // 尽早释放锁

        std::cout << "[Queue Worker] Processing task: " << task << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    InterruptibleWait waiter;
    std::vector<int> task_queue;

    std::thread t(queue_worker, std::ref(waiter), std::ref(task_queue));

    // 模拟向队列中添加任务
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lock(waiter.mtx);
        task_queue.push_back(101);
        waiter.cv.notify_one();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lock(waiter.mtx);
        task_queue.push_back(102);
        waiter.cv.notify_one();
    }

    // 等待一段时间后，中断工作线程
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "[Main Thread] Interrupting the queue worker..." << std::endl;
    waiter.interrupt();

    t.join();
    std::cout << "[Main Thread] Program finished." << std::endl;
}