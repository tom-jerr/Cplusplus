#include "../include/CLFileOP.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include "../include/CLFileOPUtils.h"
namespace neo {
namespace file {

/* file op class member funtion */
FileOP::FileOP() : wbuffer_(Buffer()), rbuffer_(Buffer()), filefd_(-1) {}

/* if open a file, close this file */
FileOP::~FileOP() {
  // flush all buffer and reset these buffers
  flushBuffer(&wbuffer_);
  flushBuffer(&rbuffer_);
  rhbuffer_.reset();

  // close file
  closeFile();
}

/* file op*/

/* open filename */
int FileOP::openFile(const char* filename) {
  filefd_ = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
  if (filefd_ == -1) {
    printf("Error: open file %s failed\n", filename);
    return -1;
  }
  return 1;
}

/* close filename*/
int FileOP::closeFile() {
  int ret;
  if (filefd_ != -1) {
    if ((ret = close(filefd_)) < 0) {
      printf("Error: close file failed\n");
      return -1;
    }
    // reset filefd
    filefd_ = -1;
  }
  return 1;
}

/* lseek file */
int FileOP::lseekFile(int mode, int offset) {
  int ret;
  if (filefd_ != -1) {
    // if lseek failed, flush buffer and try again
    if ((ret = lseek(filefd_, offset, mode)) < 0) {
      flushBuffer(&wbuffer_);

      // try again
      if ((ret = lseek(filefd_, offset, mode)) < 0) {
        printf("Error: lseek file failed\n");
        return -1;
      }
    }
  } else {
    printf("Error: no file opened, lseek failed\n");
    return -1;
  }
  return 1;
}

/* file op */

/* flush buffer to file */
int FileOP::flushBuffer(Buffer* buffer) {
  int ret;
  if (filefd_ != -1) {
    /* 将整个buffer刷到磁盘上 */
    if ((ret = write(filefd_, buffer->begin(), BUFFER_SIZE - buffer->avail())) <
        0) {
      printf("Error: flush buffer to file failed\n");
      return -1;
    }
    /* 重新设置buffer */
    buffer->reset();
  } else {
    printf("Error: no file opened, flush buffer failed\n");
    return -1;
  }
  return 1;
}

/* write file to buffer */
int FileOP::writeFile(const char* filestr) {
  int len = strlen(filestr);
  /* if avail less than buffer avail, flush buffer and reset buffer */
  if (len > wbuffer_.avail()) {
    printf("Info: buffer is full, flush buffer\n");
    flushBuffer(&wbuffer_);
  }
  /* write filestr to buffer */
  wbuffer_.append(filestr, len);
  return 1;
}

/* read file from disk to buffer */
int FileOP::readFile(char* dst, int len, int mode, int offset) {
  int ret;
  int buf_read_size = 2 * len;
  /* read double len from file */
  char* buffer = new char[buf_read_size];
  Header* head = new Header;

  if (filefd_ != -1) {
    // // first find in wbuffer, because maybe it is not flush to disk
    // for (int i = 0; i < BUFFER_SIZE - wbuffer_.avail(); i++) {
    //   if (wbuffer_.begin()[i] == offset) {
    //     memcpy(dst, wbuffer_.begin() + i, len);
    //     return 1;
    //   }
    // }

    // simple solution, just before read flush wbuffer
    flushBuffer(&wbuffer_);

    /* 如果rhbuffer中有数据，就直接从rhbuffer中读取 */
    if (rhbuffer_.getCount() > 0) {
      for (int i = 0; i < rhbuffer_.getCount(); i++) {
        if (rhbuffer_.getHeader()[i].file_off <= offset &&
            rhbuffer_.getHeader()[i].file_off +
                    rhbuffer_.getHeader()[i].data_len >=
                offset + len) {
          int r_data_off = offset - rhbuffer_.getHeader()[i].file_off;
          memcpy(dst, rbuffer_.begin() + r_data_off, len);
          return 1;
        }
      }
    }

    /* 执行从盘中读数据操作 */
    lseekFile(mode, offset);
    /* 读取2*len个字节到buffer中 */
    if ((ret = read(filefd_, buffer, 2 * len)) < 0) {
      printf("Error: read file from file failed\n");
      return -1;
    }
    /* store metadata read from disk */
    head->data_len = buf_read_size;
    head->file_off = offset;

    /* 如果rbuffer已经满了，就直接刷新rbuffer，rhbuffer直接重置即可 */
    if (len > rbuffer_.avail()) {
      printf("Info: buffer is full, flush buffer\n");
      flushBuffer(&rbuffer_);
      rhbuffer_.reset();
    }

    /* 读取到的数据放入缓存中 */
    /* metadata */
    rhbuffer_.append(head);
    /* data*/
    rbuffer_.append(buffer, strlen(dst));

    /* dst data stored*/
    memcpy(dst, buffer, len);

    /* free buffer and head */
    delete[] buffer;
    delete head;
    return 1;
  }
  printf("Error: no file opened, read file failed\n");
  return -1;
}
}  // namespace file
}  // namespace neo