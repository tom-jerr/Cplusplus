#ifndef CLUTILS_H_
#define CLUTILS_H_
// #include <cstdint>
#include <cstdio>
#include <string>
namespace neo {
class Utils {
 public:
  Utils() = default;
  ~Utils() = default;
  // void write_data(FILE *f, uint64_t a[], int n);
  // void read_data(FILE *f, uint64_t a[], int n);
  static void sort(const std::string &path, const std::string &tmpfile);
  static void merge(const std::string &path1, const std::string &path2);
};
}  // namespace neo
#endif  // CLUTILS_H_