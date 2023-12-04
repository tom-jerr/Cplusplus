#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include "../include/CLThreadPool.h"

int main(int argc, char **argv) {
  neo::ThreadPool p(2 /* two threads in the pool */);
  p.enqueue_file("../data/1.txt");
  p.enqueue_file("../data/2.txt");
  // p.enqueue_task(p.sort, "../data/1.txt", "../data/1.tmp");
  // p.enqueue(first, 1);   // function
  // p.enqueue(aga, 7, 2);  // function

  // p.enqueue(mmm, 3, "worked");

  return 0;
}