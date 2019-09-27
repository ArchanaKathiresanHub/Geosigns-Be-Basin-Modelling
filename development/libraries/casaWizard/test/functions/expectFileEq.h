// Function to compare two files line by line.
// The first are asserts, because it does not make sense to continue on an error,
// the latter are expects, because multiple mismatches should be reported.
#pragma once

#include <gtest/gtest.h>

#include <string>
#include <fstream>

void expectFileEq(const std::string& filename1, const std::string& filename2)
{
  ASSERT_NE(filename1, filename2) << "You are checking a file with itself";

  std::ifstream file1{filename1};
  std::ifstream file2{filename2};

  ASSERT_TRUE(file1.good()) << "File " << filename1 << " could not be opened";
  ASSERT_TRUE(file2.good()) << "File " << filename2 << " could not be opened";

  std::string lineFile1;
  std::string lineFile2;
  bool keepReading{true};
  int lineNumber{0};
  while (keepReading)
  {
    ++lineNumber;
    bool got1 = std::getline(file1, lineFile1).good();
    bool got2 = std::getline(file2, lineFile2).good();
    keepReading = got1 && got2;

    EXPECT_EQ(lineFile1, lineFile2) << "Mismatch at line " << lineNumber;
  }

  EXPECT_TRUE(file1.eof()) << "File " << filename1 << " not at end";
  EXPECT_TRUE(file2.eof()) << "File " << filename2 << " not at end";
}
