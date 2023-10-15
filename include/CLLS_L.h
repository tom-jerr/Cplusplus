#ifndef CLLS_L_H_
#define CLLS_L_H_
#include <fcntl.h>
#include <unistd.h>

class LS_L {
 public:
  explicit LS_L(const char* dir = ".");
  ~LS_L() = default;
  int ls();
  int ls_l();

 private:
  const char* dir_;
};
#endif  // CLLS_L_H_