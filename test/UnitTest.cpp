#include <fcntl.h>
#include <gtest/gtest.h>

#include "../include/CLFileOP.h"

#define FILENAME "test.txt"
class FileOPTest : public ::testing::Test {
 protected:
  virtual void SetUp() { file_.openFile(FILENAME); }
  virtual void TearDown() { file_.closeFile(); }
  neo::file::FileOP file_;
};

TEST_F(FileOPTest, open_close_file) { EXPECT_EQ(file_.getFileFd(), 3); }
TEST_F(FileOPTest, lseek_file) { EXPECT_EQ(file_.lseekFile(), 1); }

TEST_F(FileOPTest, write_file) {
  file_.writeFile("test");
  file_.flushBuffer(file_.getWBuffer());
  file_.lseekFile();
  char dst[5] = "\0";
  read(file_.getFileFd(), dst, 4);
  EXPECT_EQ(strcmp(dst, "test"), 0);
}

TEST_F(FileOPTest, read_file) {
  file_.lseekFile();
  char dst[5] = "\0";
  file_.readFile(dst, 4);
  EXPECT_EQ(strcmp(dst, "test"), 0);
}
