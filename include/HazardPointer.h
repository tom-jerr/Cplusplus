#pragma once
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

// ===================================================================
// 1. Hazard Pointer System
// ===================================================================

// 全局最大线程数，用于分配风险指针数组
const unsigned int MAX_THREADS = 100;

// 风险指针的持有者类，每个线程将拥有一个实例
// RAII 风格，构造时获取一个风险指针槽，析构时释放
class HazardPointer {
public:
    HazardPointer(const HazardPointer &) = delete;
    HazardPointer &operator=(const HazardPointer &) = delete;

    HazardPointer() {
        // 在全局风险指针列表中为当前线程找到一个未被使用的槽
        for (unsigned i = 0; i < MAX_THREADS; ++i) {
            if (!g_hp_owner_flags[i].test_and_set(std::memory_order_acquire)) {
                hp_slot_ = &g_hazard_pointers[i];
                owner_flag_ = &g_hp_owner_flags[i];
                return;
            }
        }
        throw std::runtime_error("Exceeded maximum number of threads");
    }

    ~HazardPointer() {
        // 释放风险指针槽
        if (hp_slot_) {
            clear();
            owner_flag_->clear(std::memory_order_release);
        }
    }

    // "宣告"一个指针有风险
    template <typename T> void protect(const std::atomic<T *> &ptr) {
        protected_ptr_ = ptr.load(std::memory_order_relaxed);
        hp_slot_->store(protected_ptr_, std::memory_order_release);
    }

    // 获取被保护的指针
    void *get() const { return protected_ptr_; }

    // 清除风险宣告
    void clear() {
        hp_slot_->store(nullptr, std::memory_order_release);
        protected_ptr_ = nullptr;
    }
    // 全局风险指针数组，存储每个线程的风险指针
    static std::atomic<void *> g_hazard_pointers[MAX_THREADS];
    // 全局标志，记录哪个槽被哪个线程占用
    static std::atomic_flag g_hp_owner_flags[MAX_THREADS];

private:
    std::atomic<void *> *hp_slot_ = nullptr;
    std::atomic_flag *owner_flag_ = nullptr;
    void *protected_ptr_ = nullptr; // 本地缓存，避免多次load
};
std::vector<void *> get_all_hazard_pointers();