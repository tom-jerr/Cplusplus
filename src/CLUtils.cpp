#include "../include/CLUtils.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
// #include <iostream>
namespace neo {

// void Utils::write_data(FILE *f, uint64_t a[], int n) {
//   for (int i = 0; i < n; ++i) fprintf(f, "%ld ", a[i]);
// }

// void Utils::read_data(FILE *f, uint64_t a[], int n) {
//   for (int i = 0; i < n; ++i) fscanf(f, "%ld", &a[i]);
// }

void Utils::sort(const std::string &path, const std::string &tmpfile) {
  FILE *fp = fopen(path.c_str(), "rb");
  FILE *tmp = fopen(tmpfile.c_str(), "wb+");
  FILE *fp1 = fopen(path.c_str(), "rb");
  fseek(fp1, 0, SEEK_END);
  int len = ftell(fp1) / sizeof(uint64_t);
  fseek(fp1, 0, SEEK_SET);
  uint64_t *buf = (uint64_t *)malloc(len * sizeof(uint64_t));
  // read_data(fp, buf, len);
  fread(buf, sizeof(uint64_t), len, fp);

  std::sort(buf, buf + len);

  // write_data(tmp, buf, len);
  fwrite(buf, sizeof(uint64_t), len, tmp);
  fclose(fp);
  fclose(fp1);
  fclose(tmp);
  free(buf);
}

void Utils::merge(const std::string &path1, const std::string &path2) {
  FILE *fp1 = fopen(path1.c_str(), "rb");
  FILE *fp2 = fopen(path2.c_str(), "rb");
  std::string path3 = "merge" + path1;
  FILE *fp3 = fopen(path3.c_str(), "wb+");
  fseek(fp1, 0, SEEK_END);
  int len1 = ftell(fp1) / sizeof(uint64_t);
  fseek(fp1, 0, SEEK_SET);
  fseek(fp2, 0, SEEK_END);
  int len2 = ftell(fp2) / sizeof(uint64_t);
  fseek(fp2, 0, SEEK_SET);
  uint64_t *buf1 = (uint64_t *)malloc(len1 * sizeof(uint64_t));
  uint64_t *buf2 = (uint64_t *)malloc(len2 * sizeof(uint64_t));
  uint64_t *buf3 = (uint64_t *)malloc((len1 + len2) * sizeof(uint64_t));
  // read_data(fp1, buf1, len1);
  // read_data(fp2, buf2, len2);
  fread(buf1, sizeof(uint64_t), len1, fp1);
  fread(buf2, sizeof(uint64_t), len2, fp2);
  int i = 0, j = 0, k = 0;
  while (i < len1 && j < len2) {
    if (buf1[i] < buf2[j])
      buf3[k++] = buf1[i++];
    else
      buf3[k++] = buf2[j++];
  }
  while (i < len1) buf3[k++] = buf1[i++];
  while (j < len2) buf3[k++] = buf2[j++];
  // write_data(fp3, buf3, len1 + len2);
  fwrite(buf3, sizeof(uint64_t), len1 + len2, fp3);
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
  free(buf1);
  free(buf2);
  free(buf3);
}
}  // namespace neo
