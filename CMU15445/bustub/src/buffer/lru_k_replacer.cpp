//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include "binder/statement/create_statement.h"
#include "common/macros.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : curr_size_(0), replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  // 先加入的timestamp在后面
  // 先删除历史队列中的
  for (auto it = --history_list_.end(); it != --history_list_.begin(); --it) {
    if (is_evictable_[*it]) {
      *frame_id = *it;
      history_list_.erase(history_map_[*frame_id]);
      history_map_.erase(*frame_id);
      curr_size_--;

      return true;
    }
  }
  // 如果都出现超过K次，从cache_list_中删除
  for (auto it = --cache_list_.end(); it != --cache_list_.begin(); --it) {
    if (is_evictable_[*it]) {
      *frame_id = *it;
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
  // 已经在cache列表中
  auto it = cache_list_.begin();
  for (; it != cache_list_.end(); ++it) {
    if (*it == frame_id) {
      access_count_[frame_id]++;

      // 每次把最新访问的放在cache_list_的最前面
      cache_list_.erase(cache_map_[frame_id]);
      cache_list_.emplace_front(frame_id);
      cache_map_[frame_id] = cache_list_.begin();

      return;
    }
  }
  // 已经在history列表中
  it = history_list_.begin();
  for (; it != history_list_.end(); ++it) {
    if (*it == frame_id) {
      access_count_[frame_id]++;
      // 已经超过K次访问
      if (access_count_[frame_id] >= k_) {
        history_list_.erase(history_map_[frame_id]);
        history_map_.erase(frame_id);
        cache_list_.emplace_front(frame_id);
        cache_map_.emplace(frame_id, cache_list_.begin());
      } else {
        // 每次把最新访问的放在history_list_的最前面
        history_list_.erase(history_map_[frame_id]);
        history_list_.emplace_front(frame_id);
        history_map_[frame_id] = history_list_.begin();
      }
      return;
    }
  }

  // 不在任何列表中，初始化访问次数为1
  access_count_.emplace(frame_id, 1);
  is_evictable_.emplace(frame_id, true);
  curr_size_++;
  history_list_.emplace_front(frame_id);
  history_map_.emplace(frame_id, history_list_.begin());
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  BUSTUB_ASSERT(frame_id <= static_cast<int>(replacer_size_), "frame_id larger than replacer size\n");
  std::scoped_lock<std::mutex> lock(latch_);
  if (is_evictable_.find(frame_id) != is_evictable_.end()) {
    if (is_evictable_[frame_id] && !set_evictable) {
      curr_size_--;
    } else if (!is_evictable_[frame_id] && set_evictable) {
      curr_size_++;
    }
    is_evictable_[frame_id] = set_evictable;
    return;
  }
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);
  // 先找cache list
  for (auto it = cache_list_.begin(); it != cache_list_.end(); ++it) {
    if (*it == frame_id) {
      cache_list_.erase(cache_map_[frame_id]);
      cache_map_.erase(frame_id);
      curr_size_--;
      return;
    }
  }
  // 再找history list
  for (auto it = history_list_.begin(); it != history_list_.end(); ++it) {
    if (*it == frame_id) {
      history_list_.erase(history_map_[frame_id]);
      history_map_.erase(frame_id);
      curr_size_--;
      return;
    }
  }
}

auto LRUKReplacer::Size() -> size_t {
  std::scoped_lock<std::mutex> lock(latch_);
  return curr_size_;
}

}  // namespace bustub
