//
// Created by 我 on 2023/7/26.
//
#include "CentralCache.hpp"
#include "PageCache.hpp"

CentralCache CentralCache::cache_;

Span* CentralCache::GetOneSpan(SpanList &spanlist, size_t size) {
    // 从span双向链表中找；如果有则直接返回
    Span* span = spanlist.Begin();
    while(span != spanlist.End()) {
        if(span->objlist_ != nullptr)
            return span;
        else
            span = span->next_;
    }

    // span列表中没有可用的span，从page cache中获取新的span
    Span* newspan = PageCache::GetInstence()->NewSpan(ClassSize::RoundUp(size));

    // 计算分配的span的起始地址和结束地址
    char* cur = (char*)(newspan->pageid_ << PAGE_SHIFT);
    char* end = cur + (newspan->npage_ << PAGE_SHIFT);
    newspan->objlist_ = cur;
    newspan->objsize_ = size;
    // split span to objects
    // 尾插法
    while(cur + size < end) {
        char* next = cur + size;
        NEXT_OBJ(cur) = next;
        cur = next;
    }
    NEXT_OBJ(cur) = nullptr;
    // 插入非空span，因为后面的span都被占用，为了查找效率，头插法
    spanlist.PushFront(newspan);

    return newspan;
}

size_t CentralCache::FetchRangObj(void *&start, void *&end, size_t n, size_t byte) {
    size_t index = ClassSize::Index(byte);
    SpanList& spanList = spanlist_[index];

    // Lock
    std::unique_lock<std::mutex> lock(spanList.mutex_);

    Span* span = GetOneSpan(spanList, byte);
    assert(span);
    assert(span->objlist_);

    size_t batchsize = 0;
    // 前一个对象
    void* prev = nullptr;
    void* cur = span->objlist_;
    // 尾插法插入n个对象
    for(size_t i = 0; i < n; ++i) {
        prev = cur;
        cur = NEXT_OBJ(cur);
        ++batchsize;
        if(cur == nullptr)
            break;
    }

    start = span->objlist_;
    end = prev;

    span->objlist_ = cur;
    span->usecount_ += batchsize;

    // 将空的span移到最后
    if(span->objlist_ == nullptr) {
        spanList.Erase(span);
        spanList.PushBack(span);
    }

    return batchsize;
}

void CentralCache::ReleaseListToSpans(void *start, size_t size) {
    size_t index = ClassSize::Index(size);
    SpanList& spanlist = spanlist_[index];

    // Lock: 自动上锁，作用域结束自动解锁
    std::unique_lock<std::mutex> lock(spanlist.mutex_);

    // 头插法将start开始的链表插入span的objlist_中
    while(start) {
        void* next = NEXT_OBJ(start);
        // map object to span
        Span* span = PageCache::GetInstence()->MapObjToSpan(start);
        NEXT_OBJ(start) = span->objlist_;
        span->objlist_ = start;

        if(--span->usecount_ == 0) {
            // 当span的对象全部归还，将span还给page cache并尝试合并
            spanlist.Erase(span);
            // page cache merge span
            PageCache::GetInstence()->ReleaseSpanToPage(span);
        }

        start = next;
    }
}