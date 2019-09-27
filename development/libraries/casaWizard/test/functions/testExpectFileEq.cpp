#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST(TestExpectFileEq, TestTwoEqualFiles)
{
  const std::string file1{"testExpectFile1.txt"};
  const std::string file2{"testExpectFile2.txt"};
  expectFileEq(file1, file2);
}
