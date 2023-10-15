#include "../include/CLLS_L.h"

#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <vector>

/* 获取目录中文件的条目 */
std::vector<struct dirent*> getEntries(const char* dir) {
  std::vector<struct dirent*> entries;
  DIR* currdir;
  currdir = opendir(dir);
  if (currdir == nullptr) {
    printf("Error: opendir %s failed\n", dir);
    return entries;
  }
  /* read entry in currdir */
  while (true) {
    struct dirent* entry;
    entry = readdir(currdir);
    if (entry == nullptr) {
      break;
    }
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    entries.push_back(entry);
  }
  return entries;
}

/* 打印lstat的参数 */
void coutInfo(struct stat statbuf, struct dirent* entry) {
  /* 打印文件类型 */
  if (S_ISREG(statbuf.st_mode)) {
    printf("-");
  } else if (S_ISDIR(statbuf.st_mode)) {
    printf("d");
  } else if (S_ISCHR(statbuf.st_mode)) {
    printf("c");
  } else if (S_ISBLK(statbuf.st_mode)) {
    printf("b");
  } else if (S_ISFIFO(statbuf.st_mode)) {
    printf("f");
  } else if (S_ISLNK(statbuf.st_mode)) {
    printf("l");
  } else if (S_ISSOCK(statbuf.st_mode)) {
    printf("s");
  }
  /* 打印文件权限 */
  printf("%c%c%c%c%c%c%c%c%c ", statbuf.st_mode & S_IRUSR ? 'r' : '-',
         statbuf.st_mode & S_IWUSR ? 'w' : '-',
         statbuf.st_mode & S_IXUSR ? 'x' : '-',
         statbuf.st_mode & S_IRGRP ? 'r' : '-',
         statbuf.st_mode & S_IWGRP ? 'w' : '-',
         statbuf.st_mode & S_IXGRP ? 'x' : '-',
         statbuf.st_mode & S_IROTH ? 'r' : '-',
         statbuf.st_mode & S_IWOTH ? 'w' : '-',
         statbuf.st_mode & S_IXOTH ? 'x' : '-');
  /* 打印文件硬链接数 */
  printf("%d ", statbuf.st_nlink);
  /* 打印文件所有者 */
  printf("%d ", statbuf.st_uid);
  /* 打印文件所属组 */
  printf("%d ", statbuf.st_gid);
  /* 打印文件大小 */
  printf("%ld ", statbuf.st_size);
  /* 打印文件最后修改时间 */
  printf("%s", ctime(&statbuf.st_mtime));
  /* 打印文件名 */
  printf("%s\n", entry->d_name);
}
LS_L::LS_L(const char* dir) : dir_(dir) {}

/* 无参数的ls */
int LS_L::ls() {
  int ret;
  /* 切换到dir_文件夹 */
  if ((ret = chdir(dir_)) < 0) {
    printf("Error: chdir %s failed\n", dir_);
    return -1;
  }

  /* 获取文件条目 */
  std::vector<struct dirent*> entries;
  entries = getEntries(dir_);
  for (auto entry : entries) {
    std::cout << entry->d_name << " ";
  }
  std::cout << std::endl;
  return 1;
}

/* ls -l */
int LS_L::ls_l() {
  int ret;
  /* 目录占用块数 */
  int total_num = 0;
  /* 切换到dir_文件夹 */
  if ((ret = chdir(dir_)) < 0) {
    printf("Error: chdir %s failed\n", dir_);
    return -1;
  }

  /* 获取文件条目 */
  std::vector<struct dirent*> entries;
  entries = getEntries(dir_);

  /* lstat获取文件信息 */
  for (auto entry : entries) {
    struct stat statbuf;
    if (lstat(entry->d_name, &statbuf) < 0) {
      printf("Error: lstat %s failed\n", entry->d_name);
      return -1;
    }
    /* 计算目录项所用的总的磁盘块数 */
    total_num += statbuf.st_blocks / 2;
  }
  std::cout << "total " << total_num << std::endl;
  /* 输出所有目录项的详细信息 */
  for (auto entry : entries) {
    struct stat statbuf;
    if (lstat(entry->d_name, &statbuf) < 0) {
      printf("Error: lstat %s failed\n", entry->d_name);
      return -1;
    }
    coutInfo(statbuf, entry);
  }
  return 1;
}

int main() {
  LS_L ls_l;
  ls_l.ls();
  ls_l.ls_l();
  return 0;
}