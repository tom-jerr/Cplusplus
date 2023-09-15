//
// Created by 我 on 2023/7/26.
//

#ifndef MYMALLOC_CENTRALCACHE_HPP
#define MYMALLOC_CENTRALCACHE_HPP
#include "Common.hpp"
/**
 *  全局使用一个central cache；使用单例模式，创建一个静态对象
 */
class CentralCache {
public:
    static CentralCache* GetInstance() {
        return &cache_;
    }

    /**
     *  get a span from page cache;
     *  1.先在现有的span双向链表中查找，如果存在free的objlist_,直接返回该span；
     *  2.如果没有；向page cache申请一个新的span，将span的连续内存切分成obj size大小加入objlist_；
     *  3.最后将新的span加入span list；使用头插法，加快查找效率
     * @param spanlist  insert to which spanlist
     * @param size      size of object stored in span
     * @return
     */
    Span* GetOneSpan(SpanList& spanlist, size_t size);

    /**
     *  从中心缓存获取一定数量的对象给thread cache；请求n个，如果不够n个有多少拿多少
     *  1.先GetOneSpan，将[start. end)的内存对象插入到objlist_中
     *  2.将空的span移到span list的后面
     * @param start     内存开始位置
     * @param end       内存结束位置
     * @param n         n个对象
     * @param byte      obj size
     * @return
     */
    size_t FetchRangObj(void*& start, void*& end, size_t n, size_t byte);

    /**
     *  将一定数量的对象释放给span list
     *  1.头插法将start开始的链表插入span的objlist_中
     *  2.如果usercount_为0，说明span中的对象都空闲；返回给page cache，进行前后页的合并
     * @param start     mem start
     * @param size      obj size
     */
    void ReleaseListToSpans(void* start, size_t size);
private:
    // 240个span链表去存储objlist_
    SpanList spanlist_[NLISTS];
private:
    CentralCache(){}
    CentralCache(const CentralCache&) = delete;
    CentralCache& operator=(const CentralCache&) = delete;

    static CentralCache cache_;
};
#endif //MYMALLOC_CENTRALCACHE_HPP
