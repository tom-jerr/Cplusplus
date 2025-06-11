#pragma once
#include "HazardPointer.h"
#include <memory>
// ===================================================================
// 2. Lock-Free Stack using Hazard Pointers
// ===================================================================

template <typename T> class LockFreeStack {
private:
    struct Node {
        T data;
        Node *next;
        Node(const T &data) : data(data), next(nullptr) {}
    };

    std::atomic<Node *> head_;

    // 每个线程都有自己的待删除列表
    static thread_local std::vector<Node *> retired_list_;

    // 尝试进行内存回收
    static void try_reclaim() {
        if (retired_list_.empty())
            return;

        // 1. 获取所有当前被保护的指针
        std::vector<void *> all_hps = get_all_hazard_pointers();

        // 2. 遍历待删除列表
        std::vector<Node *> not_reclaimed;
        for (Node *node : retired_list_) {
            bool is_hazardous = false;
            // 3. 检查节点是否在风险指针列表中
            for (void *p : all_hps) {
                if (node == p) {
                    is_hazardous = true;
                    break;
                }
            }
            // 4. 如果无风险，则删除；否则，放回待处理列表
            if (is_hazardous) {
                not_reclaimed.push_back(node);
            } else {
                delete node;
            }
        }
        // 更新待删除列表
        retired_list_.swap(not_reclaimed);
    }

public:
    LockFreeStack() : head_(nullptr) {}

    ~LockFreeStack() {
        // 注意：在实际应用中，析构时需要更复杂的逻辑来确保所有节点都被回收
        // 这里为了简化，我们假设在析构时没有其他线程在操作栈
        try_reclaim();
        for (Node *node : retired_list_) {
            delete node;
        }
        while (Node *node = head_.load()) {
            head_.store(node->next);
            delete node;
        }
    }

    void push(const T &data) {
        Node *new_node = new Node(data);
        new_node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(new_node->next, new_node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed))
            ;
    }

    std::shared_ptr<T> pop() {
        // 每个线程的风险指针实例
        thread_local HazardPointer hp;

        Node *old_head;
        while (true) {
            // 1. "宣告" 对栈顶有风险
            hp.protect(head_);
            old_head = static_cast<Node *>(hp.get());

            // 2. 验证：在我宣告之后，栈顶是否已改变？
            // 这是解决 ABA 问题的关键一步。
            if (head_.load(std::memory_order_acquire) != old_head) {
                // 验证失败，重试
                continue;
            }

            // 如果栈为空
            if (old_head == nullptr) {
                hp.clear();
                return nullptr;
            }

            // 3. 尝试原子地更新栈顶
            if (head_.compare_exchange_strong(old_head, old_head->next,
                                              std::memory_order_release,
                                              std::memory_order_relaxed)) {
                // CAS 成功，我们已经逻辑上移除了 old_head
                break; // 成功，跳出循环
            }
            // CAS 失败，说明有其他线程抢先了，循环将重试
        }

        // 4. 清除风险宣告，因为我们不再直接访问栈结构了
        hp.clear();

        // 将成功 pop 的节点放入待删除列表
        retired_list_.push_back(old_head);

        // 尝试清理（可以根据策略调整调用时机）
        if (retired_list_.size() > 2 * MAX_THREADS) {
            try_reclaim();
        }

        // 返回弹出的数据
        return std::make_shared<T>(old_head->data);
    }
};

// 静态 thread_local 变量的定义
template <typename T>
thread_local std::vector<typename LockFreeStack<T>::Node *>
    LockFreeStack<T>::retired_list_;