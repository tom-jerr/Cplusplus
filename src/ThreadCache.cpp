#include "ThreadCache.hpp"
#include "CentralCache.hpp"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) {
    FreeList* freelist = &freelist_[index];
    // 不是每次申请10个，而是进行慢增长的过程
    // 单个对象越小，申请内存块的数量越多
    // 单个对象越大，申请内存块的数量越小
    // 申请次数越多，数量多
    // 次数少,数量少
    size_t maxsize = freelist->MaxSize();
    size_t numtomove = std::min(ClassSize::NumMoveSize(size), maxsize);

    void* start = nullptr, *end = nullptr;
    // start，end分别表示取出来的内存的开始地址和结束地址
    // 取出来的内存是一个链在一起的内存对象，需要首尾标识

    // batchsize表示实际取出来的内存的个数
    // batchsize有可能小于num，表示中心缓存没有那么多大小的内存块
    size_t batchsize = CentralCache::GetInstance()->FetchRangObj(start, end, numtomove, size);

    if (batchsize > 1)
        // 避开spanlist的头节点
        freelist->PushRange(NEXT_OBJ(start), end, batchsize - 1);

    if (batchsize >= freelist->MaxSize())
        freelist->SetMaxSize(maxsize + 1);

    return start;
}

void* ThreadCache::Allocate(size_t byte) {
    assert(byte < MAXBYTES);
    // align
    byte = ClassSize::RoundUp(byte);
    size_t index = ClassSize::Index(byte);

    FreeList* list = &freelist_[index];
    if(!list->empty()) {
        return list->Pop();
    }
    else {
        // 从CentralCache中拿内存
        return FetchFromCentralCache(index, ClassSize::RoundUp(byte));
    }
}

void ThreadCache::Deallocate(void *ptr, size_t size) {
    assert(size < MAXBYTES);
    size_t index = ClassSize::Index(size);
    FreeList* list = &freelist_[index];

    list->Push(ptr);
    if(list->Size() >= list->MaxSize()) {
        ListTooLong(list, size);
    }
}

void ThreadCache::ListTooLong(FreeList *freelist, size_t byte) {
    void* start = freelist->Clear();
    // 从start开始的内存归还给中心缓存
    CentralCache::GetInstance()->ReleaseListToSpans(start, byte);
}
