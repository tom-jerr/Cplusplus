#ifndef COMMON_H_
#define COMMON_H_

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

// 定义常量
const size_t NLISTS = 240;          // 自由链表数组大小
const size_t MAXBYTES = 64 * 1024;  // ThreadCache最大分配大小
const size_t PAGE_SHIFT = 12;       // 右移位数，2^12 = 4096
const size_t NPAGES = 129;          // PageCache最大存放NPAGES-1页

// 取出该指针的前4或者前8个字节
static inline void*& NEXT_OBJ(void* obj) { return *(static_cast<void**>(obj)); }

// 链表
class FreeList {
 public:
  bool empty() { return list_ == nullptr; }

  /**
   *  insert a mem from start to end to list front
   *  (start->end->old_list; start is new list_)
   * @param start
   * @param end
   * @param num
   */
  void PushRange(void* start, void* end, size_t num) {
    NEXT_OBJ(end) = list_;
    list_ = start;
    size_ += num;
  }

  /**
   *  return element front of list
   * @return
   */
  void* Pop() {
    void* obj = list_;
    list_ = NEXT_OBJ(obj);
    size_--;
    return obj;
  }

  /**
   *  push obj to list front
   * @param obj
   */
  void Push(void* obj) {
    NEXT_OBJ(obj) = list_;
    list_ = obj;
    size_++;
  }

  size_t MaxSize() { return maxsize_; }

  void SetMaxSize(size_t size) { maxsize_ = size; }

  size_t Size() { return size_; }

  /**
   *  clear list_ and return old list_
   * @return
   */
  void* Clear() {
    size_ = 0;
    void* old_list = list_;
    list_ = nullptr;
    return old_list;
  }

  /**
   *  return reference of head node of list
   * @return
   */
  void*& GetList() { return list_; }

 private:
  void* list_ = nullptr;  // head node of list
  size_t size_ = 0;       // current size of list
  size_t maxsize_ = 1;    // max size of list
};

/**
 *  define size for freelist
 */
class ClassSize {
 public:
  // align是对齐数
  static inline size_t _RoundUp(size_t size, size_t align) {
    return (size + align - 1) & ~(align - 1);
  }

  // 向上取整
  static inline size_t RoundUp(size_t size) {
    assert(size <= MAXBYTES);

    if (size <= 128) return _RoundUp(size, 8);
    if (size <= 8 * 128) return _RoundUp(size, 16);
    if (size <= 8 * 1024) return _RoundUp(size, 128);
    if (size <= 64 * 1024)
      return _RoundUp(size, 512);
    else
      return -1;
  }

  // 控制内碎片在12%左右的浪费
  //[1, 128]						8byte对齐
  // freelist[0,16) [129, 1024]					16byte对齐
  // freelist[17, 72) [1025, 8 * 1024]				64byte对齐
  // freelist[72, 128) [8 * 1024 + 1, 64 * 1024]		512byte对齐
  // freelist[128, 240) 也就是说自由链表数组只需要开辟240个空间就可以了

  // 求出在该区间的第几个
  static size_t _Index(size_t bytes, size_t align_shift) {
    // 给bytes加上对齐数减一也就是
    // 让其可以跨越到下一个自由链表的数组
    return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
  }

  // 获取自由链表的下标
  static inline size_t Index(size_t bytes) {
    assert(bytes < MAXBYTES);

    // 记录每个对齐区间中有着多少条自由链表
    static int group_array[4] = {16, 56, 56, 112};

    if (bytes <= 128)
      return _Index(bytes, 3);
    else if (bytes <= 1024)  //(8 * 128)
      return _Index(bytes - 128, 4) + group_array[0];
    else if (bytes <= 4096)  //(8 * 8 * 128)
      return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
    else if (bytes <= 8 * 128)
      return _Index(bytes - 4096, 9) + group_array[2] + group_array[1] +
             group_array[0];
    else
      return -1;
  }

  // 计算一次从中心缓存中移动多少个内存对象到ThreadCache中
  static inline size_t NumMoveSize(size_t size) {
    if (size == 0) return 0;
    int num = (int)(MAXBYTES / size);
    if (num < 2) num = 2;
    if (num >= 512) num = 512;

    return num;
  }

  // 根据size计算中心缓存要从页缓存中取多大的span对象
  static inline size_t NumMovePage(size_t size) {
    size_t num = NumMoveSize(size);
    size_t npage = (num * size) >> PAGE_SHIFT;
    if (npage == 0) npage = 1;
    return npage;
  }
};

/**
 *  span结构管理内存；一个span含有多个页
 */
typedef size_t PageID;
struct Span {
  PageID pageid_ = 0;  // id of page
  size_t npage_ = 0;   // number of pages

  Span* next_ = nullptr;  // 双向头节点循环链表
  Span* prev_ = nullptr;  // 双向头节点循环链表

  void* objlist_ = nullptr;  // 内存对象链表
  size_t objsize_ = 0;       // object size
  size_t usecount_ = 0;      // count of used object
};

/**
 *  a list of span objects (span list has head node)
 */
class SpanList {
 public:
  std::mutex mutex_;

 public:
  SpanList() {
    head_ = new Span;
    head_->next_ = head_;
    head_->prev_ = head_;
  }
  // 拷贝操作均不定义
  SpanList(const SpanList&) = delete;
  SpanList& operator=(const SpanList&) = delete;
  ~SpanList() {
    Span* cur = head_->next_;
    while (cur != head_) {
      Span* next = cur->next_;
      delete cur;
      cur = next;
    }
    delete head_;
    head_ = nullptr;
  }

  /**
   *  return the first node
   * @return
   */
  Span* Begin() { return head_->next_; }

  /**
   *  return the next node pointer of the end node
   * @return
   */
  Span* End() { return head_; }

  bool Empty() { return head_ == head_->next_; }

  /**
   *  insert new span to front of current span
   * @param cur      当前span
   * @param newspan  要插入的新span
   */
  void Insert(Span* cur, Span* newspan) {
    assert(cur);
    Span* prev = cur->prev_;

    prev->next_ = newspan;
    newspan->prev_ = prev;
    newspan->next_ = cur;
    cur->prev_ = newspan;
  }

  /**
   *  delete current span
   * @param cur
   */
  void Erase(Span* cur) {
    assert(cur != nullptr && cur != head_);

    Span* prev = cur->prev_;
    Span* next = cur->next_;

    prev->next_ = next;
    next->prev_ = prev;
  }

  void PushBack(Span* newspan) { Insert(End(), newspan); }

  void PushFront(Span* newspan) { Insert(Begin(), newspan); }

  Span* PopBack() {
    Span* span = End()->prev_;
    Erase(span);
    return span;
  }

  Span* PopFront() {
    Span* span = Begin();
    Erase(span);
    return span;
  }

  // 互斥量上锁
  void Lock() { mutex_.lock(); }
  void UnLock() { mutex_.unlock(); }

 private:
  Span* head_ = nullptr;
};
#endif