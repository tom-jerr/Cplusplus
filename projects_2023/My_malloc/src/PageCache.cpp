//
// Created by 我 on 2023/7/27.
//
#include "PageCache.hpp"

#include <unistd.h>


PageCache PageCache::pageCache_;

Span* PageCache::AllocBigObj(size_t size) {
  assert(size > MAXBYTES);

  size = ClassSize::RoundUp(size);
  size_t npage = size >> PAGE_SHIFT;
  // 分配的span页数小于最大分配的页数; 直接在链表中寻找
  if (npage < NPAGES) {
    Span* span = NewSpan(npage);
    span->objsize_ = size;
    return span;
  } else {
    // 如果申请对象大小超过128*4096 bytes，直接向堆申请
    void* ptr = malloc(npage << PAGE_SHIFT);
    if (ptr == nullptr) throw std::bad_alloc();
    Span* span = new Span;
    span->npage_ = npage;
    span->pageid_ = reinterpret_cast<PageID>(ptr) >> PAGE_SHIFT;
    span->objsize_ = npage << PAGE_SHIFT;

    idspanmap_[span->pageid_] = span;
    return span;
  }
}

void PageCache::FreeBigObj(void* ptr, Span* span) {
  size_t npage = span->objsize_ >> PAGE_SHIFT;
  if (npage < NPAGES) {
    span->objsize_ = 0;
    ReleaseSpanToPage(span);
  } else {
    idspanmap_.erase(npage);
    delete span;
    free(ptr);
  }
}

Span* PageCache::NewSpan(size_t n) {
  std::unique_lock<std::mutex> lock(mutex_);
  return _NewSpan(n);
}

Span* PageCache::_NewSpan(size_t n) {
  assert(n < NPAGES);
  if (!spanlist_[n].Empty()) return spanlist_[n].PopFront();
  for (size_t i = n + 1; i < NPAGES; ++i) {
    if (!spanlist_[i].Empty()) {
      Span* span = spanlist_[i].PopFront();
      Span* split = new Span;
      // 分裂一个新span返回；剩下的span重新插入span list
      split->pageid_ = span->pageid_;
      split->npage_ = n;
      span->pageid_ += n;
      span->npage_ -= n;

      for (size_t i = 0; i < n; ++i) {
        idspanmap_[split->pageid_ + i] = split;
      }

      spanlist_[span->npage_].PushFront(span);
      return split;
    }
  }

  // 如果此时span list数组均没有可用空间，直接向系统申请128*4096内存
  Span* newspan = new Span;
  void* ptr = malloc((NPAGES - 1) * (1 << PAGE_SHIFT));

  newspan->pageid_ = reinterpret_cast<PageID>(ptr) >> PAGE_SHIFT;
  newspan->npage_ = NPAGES - 1;

  for (size_t i = 0; i < newspan->npage_; ++i)
    idspanmap_[newspan->pageid_ + i] = newspan;

  spanlist_[newspan->npage_].PushFront(newspan);
  // 插入新的页之后，递归查找，直到返回新的span对象
  return _NewSpan(n);
}

Span* PageCache::MapObjToSpan(void* obj) {
  PageID id = reinterpret_cast<PageID>(obj) >> PAGE_SHIFT;
  auto it = idspanmap_.find(id);
  if (it != idspanmap_.end()) {
    return it->second;
  }
  assert(false);
  return nullptr;
}

void PageCache::ReleaseSpanToPage(Span* span) {
  std::unique_lock<std::mutex> lock(mutex_);
  // 内存大于128页，直接还给操作系统
  if (span->npage_ >= NPAGES) {
    void* ptr = reinterpret_cast<void*>(span->pageid_ << PAGE_SHIFT);
    idspanmap_.erase(span->pageid_);
    free(ptr);
    delete span;
    return;
  }

  // 向前合并Span
  PageID curid = span->pageid_;
  auto previt = idspanmap_.find(span->pageid_ - 1);

  // 循环去合并; 将空间都移到前面的span上去
  while (previt != idspanmap_.end()) {
    Span* prevspan = previt->second;

    // 判断前面的span的计数是不是0
    if (prevspan->usecount_ != 0) break;

    // 判断前面的span加上后面的span有没有超出NPAGES
    // 超过128页不能合并
    if (prevspan->npage_ + span->npage_ >= NPAGES) break;

    // 进行合并(连续的页，只要将span中的页数加在一起即可)
    spanlist_[prevspan->npage_].Erase(prevspan);
    prevspan->npage_ += span->npage_;
    delete (span);
    span = prevspan;

    previt = idspanmap_.find(span->pageid_ - 1);
  }

  // 找到这个span后面的span(要跳过该span)
  auto nextvit = idspanmap_.find(span->pageid_ + span->npage_);

  while (nextvit != idspanmap_.end()) {
    Span* nextspan = nextvit->second;

    // 判断后边span的计数是不是0
    if (nextspan->usecount_ != 0) break;

    // 判断前面的span加上后面的span有没有超出NPAGES
    if (nextspan->npage_ + span->npage_ >= NPAGES) break;

    // 进行合并,将后面的span从span链中删除,合并到前面的span上
    spanlist_[nextspan->npage_].Erase(nextspan);
    span->npage_ += nextspan->npage_;
    delete (nextspan);

    nextvit = idspanmap_.find(span->pageid_ + span->npage_);
  }

  // 将合并好的页都映射到新的span上
  for (size_t i = 0; i < span->npage_; i++)
    idspanmap_[span->pageid_ + i] = span;

  // 最后将合并好的span插入到span链中
  spanlist_[span->npage_].PushFront(span);
}
