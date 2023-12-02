#include <iostream>

#include "../include/CLLog.h"
int main() {
  Log* g_log = Log::getLogInstance();
  std::cout << "test begin\n";
  g_log->writeLog("success\n");
  Log::getLogInstance()->writeLog("success\n");
}