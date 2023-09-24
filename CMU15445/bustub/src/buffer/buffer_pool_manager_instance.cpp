//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"
#include <cstddef>

#include "common/config.h"
#include "common/exception.h"
#include "common/macros.h"
#include "storage/page/page.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  page_table_ = new ExtendibleHashTable<page_id_t, frame_id_t>(bucket_size_);
  replacer_ = new LRUKReplacer(pool_size, replacer_k);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }

  // TODO(students): remove this line after you have implemented the buffer pool manager
  // throw NotImplementedException(
  //     "BufferPoolManager is not implemented yet. If you have finished implementing BPM, please remove the throw "
  //     "exception line in `buffer_pool_manager_instance.cpp`.");
}

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

}  // namespace bustub
