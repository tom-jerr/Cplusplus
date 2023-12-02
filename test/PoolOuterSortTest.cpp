#include "../include/CLOuterSort.h"

void writefile(const std::string &path, int begin, int end) {
  FILE *fp = fopen(path.c_str(), "w+");

  for (int i = begin; i <= end; i++) {
    fprintf(fp, "%d ", i);
  }
  std::cout << "write file done\n";
  fclose(fp);
}

void init() {
  writefile("../data/1.txt", 1, 500);
  writefile("../data/2.txt", 501, 1000);
}
void test() {
  OuterSort outerSort({"../data/1.txt", "../data/2.txt"}, "../data/res.txt", 2);
  outerSort.run();
}

int main() { test(); }