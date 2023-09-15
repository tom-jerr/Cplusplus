//
// Created by 我 on 2023/7/25.
//

#ifndef MYMALLOC_THREADCACHE_H
#define MYMALLOC_THREADCACHE_H

#include "Common.hpp"
#include <iostream>

/**
 *  分配的空间都是取整后的，所以是可以直接分类的
 */
class ThreadCache {
friend FreeList;
public:
    /**
     *  allocate mem for thread
     * @param size
     * @return
     */
    void* Allocate(size_t size);

    /**
     *  free mem
     * @param ptr   pointer of mem
     * @param size  size of mem
     */
    void Deallocate(void* ptr, size_t size);

    /**
     *  get mem from central cache
     * @param index index of freelist
     * @param size size of requested mem
     * @return
     */
    void* FetchFromCentralCache(size_t index, size_t size);

    /**
     *  list too long, free mem to central cache
     * @param freelist
     * @param byte
     */
    void ListTooLong(FreeList* freelist, size_t byte);

    /**
     *  return reference of freelist by index
     * @param index
     * @return
     */
    FreeList& GetFreeList(size_t index) {return freelist_[index];}
private:
    FreeList freelist_[NLISTS];
};

// 静态的tls变量，每一个ThreadCache对象都有着自己的一个tls_threadcache
// _declspec(thread)相当于每一个线程都有一个属于自己的全局变量
__thread static ThreadCache* tls_threadcache = nullptr;
#endif //MYMALLOC_THREADCACHE_H
