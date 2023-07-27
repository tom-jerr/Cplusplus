//
// Created by 我 on 2023/7/27.
//

#ifndef MYMALLOC_PAGECACHE_HPP
#define MYMALLOC_PAGECACHE_HPP

#include "Common.hpp"

/**
 *  Page cache也是每个进程一个；单例模式
 */
 class PageCache {
 public:
     static PageCache* GetInstence() {
         return &pageCache_;
     }
     /**
      *     申请大对象；如果对象大小小于128*4096字节，寻找一个span，如果对象大于该大小；直接向堆申请
      *     设置span的元数据；插入PageID和span的映射表
      * @param size     申请对象的大小
      * @return
      */
     Span* AllocBigObj(size_t size);
     void FreeBigObj(void* ptr, Span* span);

     /**
      *     切出一个n页的span
      * @param n    span分配的页的数目
      * @return
      */
     Span* NewSpan(size_t n);
     Span* _NewSpan(size_t n);

     /**
      *     寻找obj表示的地址对应的span
      *     obf >> 12 = PageID
      * @param obj  address
      * @return
      */
     Span* MapObjToSpan(void* obj);

     /**
      *     将空闲的span送回Page Cache, merge other near free span
      * @param span     需要返回上层的span
      */
     void ReleaseSpanToPage(Span* span);

 private:
     SpanList spanlist_[NPAGES];
     std::unordered_map<PageID, Span*> idspanmap_;
     std::mutex mutex_;
 private:
     PageCache(){}
     PageCache(const PageCache&) = delete;
     static PageCache pageCache_;
 };
#endif //MYMALLOC_PAGECACHE_HPP
