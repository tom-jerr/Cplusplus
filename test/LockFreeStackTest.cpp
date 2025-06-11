#include "LockFreeStack.h"
#include <gtest/gtest.h>
// ===================================================================
// 3. Main function for testing
// ===================================================================
LockFreeStack<int> stack;
const int NUM_ITEMS_PER_THREAD = 10000;
std::atomic<void *> HazardPointer::g_hazard_pointers[MAX_THREADS]{};
std::atomic_flag HazardPointer::g_hp_owner_flags[MAX_THREADS]{};

// 全局函数：扫描所有风险指针，返回一个包含所有受保护指针的集合
std::vector<void *> get_all_hazard_pointers() {
    std::vector<void *> pointers;
    for (unsigned i = 0; i < MAX_THREADS; ++i) {
        void *p =
            HazardPointer::g_hazard_pointers[i].load(std::memory_order_acquire);
        if (p) {
            pointers.push_back(p);
        }
    }
    return pointers;
}

void producer() {
    for (int i = 0; i < NUM_ITEMS_PER_THREAD; ++i) {
        stack.push(i);
    }
}

void consumer(std::atomic<int> &consumed_count) {
    for (int i = 0; i < NUM_ITEMS_PER_THREAD; ++i) {
        if (stack.pop() != nullptr) {
            consumed_count.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

int main() {
    std::cout << "Testing Lock-Free Stack with Hazard Pointers..." << std::endl;

    const int num_producers = 4;
    const int num_consumers = 4;

    std::vector<std::thread> threads;
    std::atomic<int> consumed_count(0);

    for (int i = 0; i < num_producers; ++i) {
        threads.emplace_back(producer);
    }
    for (int i = 0; i < num_consumers; ++i) {
        threads.emplace_back(consumer, std::ref(consumed_count));
    }

    for (auto &t : threads) {
        t.join();
    }

    // 处理可能剩余在栈中的元素
    while (stack.pop() != nullptr) {
        consumed_count++;
    }

    std::cout << "Total items pushed: " << num_producers * NUM_ITEMS_PER_THREAD
              << std::endl;
    std::cout << "Total items consumed: " << consumed_count.load() << std::endl;

    if (consumed_count.load() == num_producers * NUM_ITEMS_PER_THREAD) {
        std::cout << "Test PASSED!" << std::endl;
    } else {
        std::cout << "Test FAILED!" << std::endl;
    }

    return 0;
}