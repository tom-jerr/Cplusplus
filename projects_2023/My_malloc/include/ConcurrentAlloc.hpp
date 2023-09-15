//
// Created by 我 on 2023/7/27.
//

#ifndef MYMALLOC_CONCURRENTALLOC_HPP
#define MYMALLOC_CONCURRENTALLOC_HPP

#include "Common.hpp"
#include "PageCache.hpp"
#include "ThreadCache.hpp"
#include "CentralCache.hpp"

//被动调用，哪个线程来了之后，需要内存就调用这个接口
static inline void* ConcurrentAlloc(size_t size)
{
    if (size > MAXBYTES)//超过一个最大值 64k，就自己从系统中获取，否则使用内存池
    {
        //return malloc(size);
        Span* span = PageCache::GetInstence()->AllocBigObj(size);
        void* ptr = (void*)(span->pageid_ << PAGE_SHIFT);
        return ptr;
    }
    else
    {
        if (tls_threadcache == nullptr)//第一次来，自己创建，后面来的，就可以直接使用当前创建好的内存池
            tls_threadcache = new ThreadCache;

        return tls_threadcache->Allocate(size);
    }
}

static inline void ConcurrentFree(void* ptr)//最后释放
{
    Span* span = PageCache::GetInstence()->MapObjToSpan(ptr);
    size_t size = span->objsize_;
    if (size > MAXBYTES)
        PageCache::GetInstence()->FreeBigObj(ptr, span);
    else
        tls_threadcache->Deallocate(ptr, size);

}
#endif //MYMALLOC_CONCURRENTALLOC_HPP
