//
// Created by 我 on 2023/7/27.
//
#include "CentralCache.hpp"
#include "Common.hpp"
#include "ConcurrentAlloc.hpp"
#include "PageCache.hpp"

using std::cout;
using std::endl;

void TestSize() {
  cout << ClassSize::RoundUp(10) << endl;
  cout << ClassSize::RoundUp(1025) << endl;
  cout << ClassSize::RoundUp(1024 * 8 + 1) << endl;

  cout << ClassSize::NumMovePage(16) << endl;
  cout << ClassSize::NumMovePage(1024) << endl;
  cout << ClassSize::NumMovePage(1024 * 8) << endl;
  cout << ClassSize::NumMovePage(1024 * 64) << endl;
}

void Alloc(size_t n) {
  size_t begin1 = clock();
  std::vector<void*> v;
  for (size_t i = 0; i < n; ++i) {
    v.push_back(ConcurrentAlloc(10));
  }

  for (size_t i = 0; i < n; ++i) {
    ConcurrentFree(v[i]);
    cout << v[i] << endl;
  }
  v.clear();
  size_t end1 = clock();

  size_t begin2 = clock();

  for (size_t i = 0; i < n; ++i) {
    v.push_back(ConcurrentAlloc(10));
  }

  for (size_t i = 0; i < n; ++i) {
    ConcurrentFree(v[i]);
    cout << v[i] << endl;
  }
  v.clear();
  size_t end2 = clock();

  // 两次耗时
  cout << endl << endl;
  cout << end1 - begin1 << endl;
  cout << end2 - begin2 << endl;
}

void TestThreadCache() {
  std::thread t1(Alloc, 2);
  // std::thread t2(Alloc, 5);
  // std::thread t3(Alloc, 5);
  // std::thread t4(Alloc, 5);

  t1.join();
  // t2.join();
  // t3.join();
  // t4.join();
}

void TestCentralCache() {
  std::vector<void*> v;
  for (size_t i = 0; i < 8; ++i) {
    v.push_back(ConcurrentAlloc(10));
  }

  for (size_t i = 0; i < 8; ++i) {
    // ConcurrentFree(v[i], 10);
    cout << v[i] << endl;
  }
}

void TestPageCache() { PageCache::GetInstence()->NewSpan(2); }

void TestConcurrentAllocFree() {
  size_t n = 2;
  std::vector<void*> v;
  for (size_t i = 0; i < n; ++i) {
    void* ptr = ConcurrentAlloc(99999);
    v.push_back(ptr);
  }

  for (size_t i = 0; i < n; ++i) {
    ConcurrentFree(v[i]);
  }
  cout << "hehe" << endl;
}

void AllocBig() {
  void* ptr1 = ConcurrentAlloc(65 << PAGE_SHIFT);
  void* ptr2 = ConcurrentAlloc(129 << PAGE_SHIFT);

  ConcurrentFree(ptr1);
  ConcurrentFree(ptr2);
}

int main() {
  //	TestSize();
  //   TestThreadCache();
  //   TestCentralCache();
  //   TestPageCache();
  //	TestConcurrentAllocFree();
  //
  //	//AllocBig();
  //	system("pause");
  return 0;
}