/*
 * CS106L Assignment 3: Make a Class
 * Created by Fabio Ibanez with modifications by Jacob Roberts-Baca.
 */
#include "class.h"
#include <iostream>
/* #### Please don't change this line! #### */
int run_autograder();

int main() {
  // STUDENT TODO: Construct an instance of your class!
  Test test(5);
  auto data = test.getData();
  std::cout << data << std::endl;
  test.setData(10);
  data = test.getData();
  std::cout << data << std::endl;

  /* #### Please don't change this line! #### */
  return run_autograder();
}

/* #### Please don't change this line! #### */
#include "utils.hpp"