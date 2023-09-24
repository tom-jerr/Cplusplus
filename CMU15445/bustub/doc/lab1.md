# Buffer Pool

## Task 1 Extendible Hash Table

### 基本概念

- 全局深度（global depth），局部深度（local depth）以及桶（bucket）。
- 桶就是存记录（records）的地方，我可以往桶里放一条，两条，三条。但要规定好最多能装几条。这个例子里我们规定最多能装2条。在图里你能看到bucket 1只有两行。下图这个就是可拓展哈希的初始状态。



![img](https://pic2.zhimg.com/80/v2-e91e24d296ed410ca1a7a512c0c17f71_720w.webp)



### 插入数据。

- 插入Comp.Sci和Finance这两条。现在还不涉及全局深度和局部深度的改变。两条新记录（record）就直接往里放就行。

![img](https://pic1.zhimg.com/80/v2-de0b9c301ec77baea44decf7beb05688_720w.webp)



- 插入Music。这时一个桶的条数大于它的上限（2条），所以要开始分裂。如何分裂是根据哈希值（hash value）决定的。取哈希值第一位。哈希值是二进制数，第一位只有两种可能，0和1。Comp.Sci是1，Finance是1，Music是0。很简单的把是1的放在一个桶（bucket）里，是0的放在另一个桶（bucket）里。由于我们用了哈希值的第一位，所以全局深度（global depth）变成1。这里做一下全局深度和局部深度的区分。两个都表示用了几位哈希值。、
- 全局深度是所有记录要用到几位哈希值，局部深度是该桶里的记录用到几位哈希值。产生这种区别的原因是，有的哈希值用不到全局深度那么多位。我们会在后面的例子看到，现在这么说还比较抽象。

![img](https://pic1.zhimg.com/80/v2-2d3849048c9f43e0a665ede6438d1100_720w.webp)

- 如果插入元素后，该bucket已满
  - 如果当前Local depth == global depth；global depth++，桶进行分裂；分裂之后产生的新桶和原来的桶local depth++
  - 如果local depth < global depth；桶进行分裂；分裂之后产生的新桶和原来的桶local depth++

### Extendible_hash_table.h

#### Bucket

- 对于bucket中的查找、增加和删除；就是对list的查找增加和删除

#### hash table
**构建哈希表时，dir_是vector，需要预先插入一个元素，否则空间还未分配就使用，产生读内存错误**

- 查找、删除通过IndexOf确定在目录中的index；直接调用Bucket的find和remove
- 插入：判断是否bucket已经满了
  - 如果当前Local depth == global depth；global depth++，桶进行分裂；
  - 如果local depth < global depth；桶进行分裂；分裂之后产生的新桶和原来的桶local depth++；重新映射原来的桶和新分裂的桶

```c++
template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Find(const K &key, V &value) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  // find the bucket whick key belongs to
  auto bucket = dir_[IndexOf(key)];
  return static_cast<bool>(bucket->Find(key, value));
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Remove(const K &key) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  // find the bucket whick key belongs to
  auto bucket = dir_[IndexOf(key)];
  return static_cast<bool>(bucket->Remove(key));
}

template <typename K, typename V>
void ExtendibleHashTable<K, V>::Insert(const K &key, const V &value) {
  //  UNREACHABLE("not implemented");
  std::scoped_lock<std::mutex> lock(latch_);
  while (true) {
    // find the bucket whick key belongs to; 扩容后index值改变
    auto bucket = dir_[IndexOf(key)];
    // 1 需要扩展哈希表
    auto flag = bucket->Insert(key, value);
    if (flag) {
      break;
    }
    // 需要增加global_depth
    if (bucket->GetDepth() == global_depth_) {
      size_t len = dir_.size();
      dir_.resize(2 * len);
      // 将原来的值拷贝进来，现在指向同一个bucket，需要处理
      for (size_t i = 0; i < len; ++i) {
        dir_[i + len] = dir_[i];
      }
      ++global_depth_;
    }

    // 2 需要buckets进行拆分
    if (bucket->GetDepth() < global_depth_) {
      bucket->IncrementDepth();
      ++num_buckets_;
      // 通过最后一个有效位来对应dir_[i]和bucket
      int mask = (1 << bucket->GetDepth()) - 1;
      // 对应的k和bucket，如果没有对应的bucket，就重新分配一个bucket
      std::unordered_map<size_t, std::shared_ptr<Bucket>> split_bucket;
      // 对0~len的dir_进行重新映射
      for (size_t i = 0; i < dir_.size(); ++i) {
        if (dir_[i] == bucket) {
          // 根据末尾的有效位来确定
          size_t k = i & mask;
          if (!split_bucket.count(k)) {
            dir_[i] = std::shared_ptr<Bucket>(new Bucket(bucket_size_, bucket->GetDepth()));
            split_bucket[k] = dir_[i];
          } else {
            dir_[i] = split_bucket[k];
          }
        }
      }

      for (auto &[key, value] : bucket->GetItems()) {
        dir_[IndexOf(key)]->Insert(key, value);
      }
    }
  }
}

//===--------------------------------------------------------------------===//
// Bucket
//===--------------------------------------------------------------------===//
template <typename K, typename V>
ExtendibleHashTable<K, V>::Bucket::Bucket(size_t array_size, int depth) : size_(array_size), depth_(depth) {}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Find(const K &key, V &value) -> bool {
  // UNREACHABLE("not implemented");
  for (auto &[k, v] : list_) {
    if (k == key) {
      value = v;
      return true;
    }
  }
  return false;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Remove(const K &key) -> bool {
  //  UNREACHABLE("not implemented");
  bool flag = std::any_of(list_.begin(), list_.end(), [key](auto it) { return key == it.first; });
  if (flag) {
    for (auto &it : list_) {
      if (it.first == key) {
        list_.remove(it);
        return true;
      }
    }
  }
  return false;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Insert(const K &key, const V &value) -> bool {
  //  UNREACHABLE("not implemented");
  // whehter full or not
  for (auto &[k, v] : list_) {
    if (key == k) {
      v = value;
      return true;
    }
  }
  if (IsFull()) {
    return false;
  }
  list_.push_back(std::make_pair(key, value));
  return true;
}
```

## Task2 LRU-K Replacement Policy

- 缓存替换策略
- 每次替换会优先替换k-距离最远的一个数
- 需要两个队列进行存储
  - 一个历史访问队列，一个缓存队列
  - 数据第一次访问加入history list，在history list中访问未达到K次；使用LRU进行删除
  - 在history list中访问达到K次，从history list中删除，插入到cache list中；
  - 淘汰策略：现在history list中找；如果没有，将在cache list中寻找

- 使用`std::list`来设计历史队列和缓存队列；因为erase删除需要list的迭代器；需要使用map来存储id和对应的迭代器`std::unordered_map<frmae_id, std::list<frame_id>::iterator>`
#### lru_k_replacer.h
```c++

  // 历史队列（list.erase需要list的迭代器；需要map存储list的迭代器）
  std::list<frame_id_t> history_list_;
  std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> history_map_;
  // 缓存队列
  std::list<frame_id_t> cache_list_;
  std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> cache_map_;

  std::unordered_map<frame_id_t, bool> is_evictable_;
```
#### lru_k_replacer.cpp
- 记得在驱逐和Remove entry时，记得将access_count和is_evictable进行初始化设置
- 否则可能在线上出现一些未知错误
- SetEvictable中，如果access_count为0时，不进行标记的设置
  
~~~c++
LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : curr_size_(0), replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  if (curr_size_ == 0) {
    return false;
  }
  // 先加入的timestamp在后面
  // 先删除历史队列中的
  for (auto it = history_list_.rbegin(); it != history_list_.rend(); ++it) {
    if (is_evictable_[*it]) {
      *frame_id = *it;
      // 移除后，记得进行初始化操作
      access_count_[*frame_id] = 0;
      is_evictable_[*frame_id] = false;
      history_list_.erase(history_map_[*frame_id]);
      history_map_.erase(*frame_id);
      curr_size_--;

      return true;
    }
  }
  // 如果都出现超过K次，从cache_list_中删除
  for (auto it = cache_list_.rbegin(); it != cache_list_.rend(); ++it) {
    if (is_evictable_[*it]) {
      *frame_id = *it;
      // 移除后，记得进行初始化操作
      access_count_[*frame_id] = 0;
      is_evictable_[*frame_id] = false;
      cache_list_.erase(cache_map_[*frame_id]);
      cache_map_.erase(*frame_id);
      curr_size_--;

      return true;
    }
  }
  return false;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
  BUSTUB_ASSERT(frame_id <= static_cast<int>(replacer_size_), "frame_id larger than replacer size\n");
  std::scoped_lock<std::mutex> lock(latch_);
  if (frame_id > static_cast<int>(replacer_size_)) {
    throw std::exception();
  }

  access_count_[frame_id]++;

  if (access_count_[frame_id] == k_) {
    auto it = history_map_[frame_id];
    history_list_.erase(it);
    history_map_.erase(frame_id);

    cache_list_.push_front(frame_id);
    cache_map_[frame_id] = cache_list_.begin();
  } else if (access_count_[frame_id] > k_) {
    if (cache_map_.count(frame_id) != 0U) {
      auto it = cache_map_[frame_id];
      cache_list_.erase(it);
    }
    cache_list_.push_front(frame_id);
    cache_map_[frame_id] = cache_list_.begin();
  } else {
    if (history_map_.count(frame_id) == 0U) {
      history_list_.push_front(frame_id);
      history_map_[frame_id] = history_list_.begin();
    }
  }
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  BUSTUB_ASSERT(frame_id <= static_cast<int>(replacer_size_), "frame_id larger than replacer size\n");
  std::scoped_lock<std::mutex> lock(latch_);
  if (frame_id > static_cast<int>(replacer_size_)) {
    throw std::exception();
  }
  // 无进程占用不进行设置
  if (access_count_[frame_id] == 0) {
    return;
  }

  if (!is_evictable_[frame_id] && set_evictable) {
    curr_size_++;
  }
  if (is_evictable_[frame_id] && !set_evictable) {
    curr_size_--;
  }
  is_evictable_[frame_id] = set_evictable;
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);
  if (frame_id > static_cast<int>(replacer_size_)) {
    throw std::exception();
  }
  auto cnt = access_count_[frame_id];
  if (cnt == 0) {
    return;
  }
  if (!is_evictable_[frame_id]) {
    throw std::exception();
  }
  if (cnt < k_) {
    history_list_.erase(history_map_[frame_id]);
    history_map_.erase(frame_id);

  } else {
    cache_list_.erase(cache_map_[frame_id]);
    cache_map_.erase(frame_id);
  }
  curr_size_--;
  // 移除后，记得进行初始化操作
  access_count_[frame_id] = 0;
  is_evictable_[frame_id] = false;
}

auto LRUKReplacer::Size() -> size_t {
  std::scoped_lock<std::mutex> lock(latch_);
  return curr_size_;
}
~~~
## Task3 Buffer Pool Manager Instance

- 从disk中取出存放的页；将其存储在内存中
- 每个页有一块内存用来存放物理页中的数据；一个计数器，记录被线程pin的次数，一旦归0，写回修改的页面，页面初始化（重新设置内存和元数据）

### Page

- is_dirty_：判断页是否需要写回磁盘
- pin_count_：被多少个线程固定
- data[]：内存存放disk中的物理页

```c++
class Page {
  // There is book-keeping information inside the page that should only be relevant to the buffer pool manager.
  friend class BufferPoolManagerInstance;

 public:
  /** Constructor. Zeros out the page data. */
  Page() { ResetMemory(); }

  /** Default destructor. */
  ~Page() = default;

  /** @return the actual data contained within this page */
  inline auto GetData() -> char * { return data_; }

  /** @return the page id of this page */
  inline auto GetPageId() -> page_id_t { return page_id_; }

  /** @return the pin count of this page */
  inline auto GetPinCount() -> int { return pin_count_; }

  /** @return true if the page in memory has been modified from the page on disk, false otherwise */
  inline auto IsDirty() -> bool { return is_dirty_; }

  /** Acquire the page write latch. */
  inline void WLatch() { rwlatch_.WLock(); }

  /** Release the page write latch. */
  inline void WUnlatch() { rwlatch_.WUnlock(); }

  /** Acquire the page read latch. */
  inline void RLatch() { rwlatch_.RLock(); }

  /** Release the page read latch. */
  inline void RUnlatch() { rwlatch_.RUnlock(); }

  /** @return the page LSN. */
  inline auto GetLSN() -> lsn_t { return *reinterpret_cast<lsn_t *>(GetData() + OFFSET_LSN); }

  /** Sets the page LSN. */
  inline void SetLSN(lsn_t lsn) { memcpy(GetData() + OFFSET_LSN, &lsn, sizeof(lsn_t)); }

 protected:
  static_assert(sizeof(page_id_t) == 4);
  static_assert(sizeof(lsn_t) == 4);

  static constexpr size_t SIZE_PAGE_HEADER = 8;
  static constexpr size_t OFFSET_PAGE_START = 0;
  static constexpr size_t OFFSET_LSN = 4;

 private:
  /** Zeroes out the data that is held within the page. */
  inline void ResetMemory() { memset(data_, OFFSET_PAGE_START, BUSTUB_PAGE_SIZE); }

  /** The actual data that is stored within a page. */
  char data_[BUSTUB_PAGE_SIZE]{};
  /** The ID of this page. */
  page_id_t page_id_ = INVALID_PAGE_ID;
  /** The pin count of this page. */
  int pin_count_ = 0;
  /** True if the page is dirty, i.e. it is different from its corresponding page on disk. */
  bool is_dirty_ = false;
  /** Page latch. */
  ReaderWriterLatch rwlatch_;
};
```

### Buffer Pool Manager

- 从磁盘上读取page插入到对应的buffer pool

#### buffer_pool_manager_instance.h

- 寻找buffer pool中空闲的frame
- 将page和frame对应插入到page_table_中；为新页重新设置内存和元数据；可扩展散列进行RecordAccess和SetEvitable；从disk中利用disk_manage\_进行磁盘数据的读取
- 写回磁盘利用disk_manager_.WritePage()

```c++
class BufferPoolManagerInstance : public BufferPoolManager {
 public:
  BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager, size_t replacer_k = LRUK_REPLACER_K,
                            LogManager *log_manager = nullptr);

  /**
   * @brief Destroy an existing BufferPoolManagerInstance.
   */
  ~BufferPoolManagerInstance() override;

  /** @brief Return the size (number of frames) of the buffer pool. */
  auto GetPoolSize() -> size_t override { return pool_size_; }

  /** @brief Return the pointer to all the pages in the buffer pool. */
  auto GetPages() -> Page * { return pages_; }

 protected:
  auto NewPgImp(page_id_t *page_id) -> Page * override;
age
  auto FetchPgImp(page_id_t page_id) -> Page * override;
  auto UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool override;
    
  auto FlushPgImp(page_id_t page_id) -> bool override;
  void FlushAllPgsImp() override;

  auto DeletePgImp(page_id_t page_id) -> bool override;

  /** Number of pages in the buffer pool. */
  const size_t pool_size_;
  /** The next page id to be allocated  */
  std::atomic<page_id_t> next_page_id_ = 0;
  /** Bucket size for the extendible hash table */
  const size_t bucket_size_ = 4;

  /** Array of buffer pool pages. */
  Page *pages_;
  /** Pointer to the disk manager. */
  DiskManager *disk_manager_ __attribute__((__unused__));
  /** Pointer to the log manager. Please ignore this for P1. */
  LogManager *log_manager_ __attribute__((__unused__));
  /** Page table for keeping track of buffer pool pages. */
  ExtendibleHashTable<page_id_t, frame_id_t> *page_table_;
  /** Replacer to find unpinned pages for replacement. */
  LRUKReplacer *replacer_;
  /** List of free frames that don't have any pages on them. */
  std::list<frame_id_t> free_list_;
  /** This latch protects shared data structures. We recommend updating this comment to describe what it protects. */
  std::mutex latch_;
  // 增加next_page_id_
  auto AllocatePage() -> page_id_t;

  void DeallocatePage(__attribute__((unused)) page_id_t page_id) {
  
  }
};
```
#### buffer_pool_manager.cpp
- 注意加锁和解锁
- 注意对TODO进行注释，否则运行测试出错

~~~c++
BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete page_table_;
  delete replacer_;
}

auto BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) -> Page * {
  latch_.lock();
  frame_id_t frame_id;
  // 寻找是否有free的frame或者可以驱逐的frame
  if (!free_list_.empty()) {
    frame_id = free_list_.front();
    free_list_.pop_front();
  } else if (!replacer_->Evict(&frame_id)) {
    // 如果不能驱逐
    page_id = nullptr;
    latch_.unlock();
    return nullptr;
  }

  // 分配一个物理上的新页
  *page_id = AllocatePage();
  Page *page = &pages_[frame_id];
  // 判断是否是脏页
  if (page->IsDirty()) {
    disk_manager_->WritePage(page->GetPageId(), page->GetData());
    page->is_dirty_ = false;
  }

  // 每次设置新页时重新memset该page
  page_table_->Remove(page->GetPageId());
  page->ResetMemory();
  page->page_id_ = *page_id;
  page->pin_count_ = 1;

  page_table_->Insert(*page_id, frame_id);
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);
  latch_.unlock();
  return page;
}

auto BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) -> Page * {
  frame_id_t frame_id;
  latch_.lock();
  if (!page_table_->Find(page_id, frame_id)) {
    // found from free list
    if (!free_list_.empty()) {
      frame_id = free_list_.front();
      free_list_.pop_front();
    }
    // found in replacer_
    else if (!replacer_->Evict(&frame_id)) {
      latch_.unlock();
      return nullptr;
    }
  } else {
    // 直接找到了Page
    replacer_->RecordAccess(frame_id);
    replacer_->SetEvictable(frame_id, false);
    Page *page = &pages_[frame_id];
    ++page->pin_count_;
    latch_.unlock();
    return &pages_[frame_id];
  }

  // 需要分配新页
  Page *page = &pages_[frame_id];
  if (page->IsDirty()) {
    disk_manager_->WritePage(page->GetPageId(), page->GetData());
    page->is_dirty_ = false;
  }

  // 移除旧页，每次设置新页时重新memset该page
  page_table_->Remove(page->GetPageId());
  page->ResetMemory();

  disk_manager_->ReadPage(page_id, page->GetData());
  page->page_id_ = page_id;
  page->pin_count_ = 1;

  page_table_->Insert(page_id, frame_id);
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);
  latch_.unlock();
  return &pages_[frame_id];
}

auto BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool {
  frame_id_t frame_id;
  latch_.lock();
  if (!page_table_->Find(page_id, frame_id)) {
    latch_.unlock();
    return false;
  }
  Page *page = &pages_[frame_id];
  // 已经不能解开pin
  if (page->GetPinCount() <= 0) {
    latch_.unlock();
    return false;
  }
  page->pin_count_--;
  // 没有进程锁住，可以被驱逐
  if (page->GetPinCount() == 0) {
    replacer_->SetEvictable(frame_id, true);
  }
  page->is_dirty_ |= is_dirty;
  latch_.unlock();
  return true;
}

auto BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) -> bool {
  frame_id_t frame_id;
  latch_.lock();
  if (page_id == INVALID_PAGE_ID) {
    latch_.unlock();
    return false;
  }
  if (!page_table_->Find(page_id, frame_id)) {
    latch_.unlock();
    return false;
  }

  Page *page = &pages_[frame_id];
  disk_manager_->WritePage(page_id, page->GetData());
  latch_.unlock();
  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  latch_.lock();
  for (size_t i = 0; i < pool_size_; ++i) {
    Page *page = &pages_[i];
    if (page->GetPageId() != INVALID_PAGE_ID) {
      disk_manager_->WritePage(page->GetPageId(), page->GetData());
      page->is_dirty_ = false;
    }
  }
  latch_.unlock();
}

auto BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) -> bool {
  frame_id_t frame_id;
  latch_.lock();
  if (!page_table_->Find(page_id, frame_id)) {
    latch_.unlock();
    return true;
  }
  Page *page = &pages_[frame_id];
  // 被锁住不能被删除
  if (page->GetPinCount() > 0) {
    latch_.unlock();
    return false;
  }
  // 从buffer pool和lru中删除，将该页帧加入空闲链表
  page_table_->Remove(page_id);
  replacer_->Remove(frame_id);
  free_list_.push_back(frame_id);
  page->ResetMemory();
  DeallocatePage(page_id);

  latch_.unlock();
  return true;
}

auto BufferPoolManagerInstance::AllocatePage() -> page_id_t { return next_page_id_++; }

~~~
