#include <iostream>

#include "CentralCache.hpp"
#include "ThreadCache.hpp"


using namespace std;
int main() {
  ThreadCache thead;
  void* ptr1 = malloc(32);
  cout << "Begin:\n";
  cout << "ptr:\t" << ptr1 << endl;
  cout << "list_\t" << thead.GetFreeList(3).GetList() << endl;

  thead.GetFreeList(3).PushRange(
      ptr1, static_cast<void*>(static_cast<char*>(ptr1) + 32), 1);
  cout << "after push range:\n";
  cout << "list_\t" << thead.GetFreeList(3).GetList() << endl;

  void* ptr = thead.Allocate(32);
  cout << "Allocate:\n";
  cout << "list_\t" << thead.GetFreeList(3).GetList() << endl;
  cout << "Pop ptr:\t" << ptr << endl;

  thead.Deallocate(ptr, 32);
  cout << "Deallocate:\n";
  cout << "list_\t" << thead.GetFreeList(3).GetList() << endl;
  return 0;
}
