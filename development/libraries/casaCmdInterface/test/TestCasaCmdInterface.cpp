#include "../src/casaCmdInterface.h"
#include <gtest/gtest.h>

using namespace casaCmdInterface;

TEST(casaCmdInterfaceTest, testcasaCmdInterface)
{
std::string testString = "test abc def ghi \"abc def kji\" mfsdk435f \"abc defkji\"";

std::string cmd;
std::vector<std::string> strVec = stringVecFromCasaCommand(testString,cmd);
std::string cmdString = cmd + " " + casaCommandFromStrVec(strVec);

EXPECT_EQ(testString,cmdString);
EXPECT_EQ(stringVecToStringWithNoSpaces(strVec,"_"),"abc_def_ghi_abc_def_kji_mfsdk435f_abc_defkji");
}
