#ifndef CLFILEOPUTILS_H_
#define CLFILEOPUTILS_H_
#include <fcntl.h>

#include "CLFileOPUtils.h"
namespace neo {
namespace file {

class FileOP {
 public:
  FileOP();
  ~FileOP();
  int getFileFd() const { return filefd_; }
  /* file update op */

  /* write File */
  int writeFile(const char* filestr);
  /* read File */
  int readFile(char* dst, int len, int offset = 0);
  /* flush buffer to file */
  /* buffer is read buffer or write buffer */
  int flushBuffer(Buffer* buffer);

  /* file other op */

  /* open filename */
  int openFile(const char* filename);
  /* close filename*/
  int closeFile();
  /* return offset in filename under mode */
  /* 默认直接从头开始 */
  int lseekFile(int mode = SEEK_SET, int offset = 0);

  /* get member op */
  Buffer* getWBuffer() { return &wbuffer_; }
  Buffer* getRBuffer() { return &rbuffer_; }
  HBuffer* getRHBuffer() { return &rhbuffer_; }

 private:
  /* store data to write into disk*/
  Buffer wbuffer_;
  /* store data read from disk */
  Buffer rbuffer_;
  // /* store metadata write to disk */
  // HBuffer whbuffer_;
  /* store metadata read from disk */
  HBuffer rhbuffer_;
  /* file descriptor*/
  int filefd_;
};
}  // namespace file
}  // namespace neo
#endif  //  CLFILEOPUTILS_H_