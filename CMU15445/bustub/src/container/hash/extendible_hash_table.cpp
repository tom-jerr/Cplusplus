//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_hash_table.cpp
//
// Identification: src/container/hash/extendible_hash_table.cpp
//
// Copyright (c) 2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdlib>
#include <functional>
#include <list>
#include <utility>

#include "container/hash/extendible_hash_table.h"
#include "storage/page/page.h"

namespace bustub {

template <typename K, typename V>
ExtendibleHashTable<K, V>::ExtendibleHashTable(size_t bucket_size)
    : global_depth_(0), bucket_size_(bucket_size), num_buckets_(1) {
  // vector需要预先分配空间
  dir_.push_back(std::shared_ptr<Bucket>(new Bucket(bucket_size_)));
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::IndexOf(const K &key) -> size_t {
  int mask = (1 << global_depth_) - 1;
  return std::hash<K>()(key) & mask;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetGlobalDepth() const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetGlobalDepthInternal();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetGlobalDepthInternal() const -> int {
  return global_depth_;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetLocalDepth(int dir_index) const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetLocalDepthInternal(dir_index);
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetLocalDepthInternal(int dir_index) const -> int {
  return dir_[dir_index]->GetDepth();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetNumBuckets() const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetNumBucketsInternal();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetNumBucketsInternal() const -> int {
  return num_buckets_;
}

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

template class ExtendibleHashTable<page_id_t, Page *>;
template class ExtendibleHashTable<Page *, std::list<Page *>::iterator>;
template class ExtendibleHashTable<int, int>;
// test purpose
template class ExtendibleHashTable<int, std::string>;
template class ExtendibleHashTable<int, std::list<int>::iterator>;

}  // namespace bustub
