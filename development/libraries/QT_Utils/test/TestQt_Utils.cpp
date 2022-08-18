//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Qt_Utils.h"

#include <gtest/gtest.h>
#include <QString>

TEST( Qt_UtilsTest, testEscapeSpecialCharacters)
{
   QString stringWithSpecialCharacters = "\\\\'\"\?\a\b\f\n\r\t\vjh";

   QString escapedString = qtutils::escapeSpecialCharacters(stringWithSpecialCharacters);
   EXPECT_EQ(escapedString.toStdString(), "\\\\'\"\?\\a\\b\\f\\n\\r\\t\\vjh");

   //Test that the escapes are not again escaped.
   QString escapedString2 = qtutils::escapeSpecialCharacters(escapedString);
   EXPECT_EQ(escapedString, escapedString2);
}

TEST(Qt_UtilsTest, testdoubleToQString)
{
   using namespace qtutils;
   QString outStr = doubleToQString(3.1415e66);
   EXPECT_EQ(outStr,"3.1415e+66");

   outStr = doubleToQString(3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679e66);
   EXPECT_EQ(outStr,"3.14159265359e+66");

   outStr = doubleToQString(0.0000000000001234);
   EXPECT_EQ(outStr,"1.234e-13");

   outStr = doubleToQString(3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679);
   EXPECT_EQ(outStr,"3.14159265359");

   outStr = doubleToQString(-3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679);
   EXPECT_EQ(outStr,"-3.14159265359");
}

TEST(Qt_UtilsTest, testdoubleOneDigitToQString)
{
   using namespace qtutils;
   QString outStr = doubleOneDigitToQString(3.1415e16);
   EXPECT_EQ(outStr,"31415000000000000.0");

   outStr = doubleOneDigitToQString(3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679e66);
   EXPECT_EQ(outStr,"3141592653589793108048005644549462632004482767434675528474212433920.0");

   outStr = doubleOneDigitToQString(0.0000000000001234);
   EXPECT_EQ(outStr,"0.0");

   outStr = doubleOneDigitToQString(3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679);
   EXPECT_EQ(outStr,"3.1");

   outStr = doubleOneDigitToQString(-3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679);
   EXPECT_EQ(outStr,"-3.1");
}

TEST(Qt_UtilsTest, testIntVecToStringList)
{
   using namespace qtutils;
   {
      QVector<int> vec{};
      QStringList list = convertToQStringList(vec);
      EXPECT_TRUE(list.size() == 0);
   }
   {
      QVector<int> vec{-1234567891};
      QStringList list = convertToQStringList(vec);
      QStringList expectedList{"-1234567891"};
      EXPECT_EQ(list,expectedList);
   }
   {
      QVector<int> vec{-1234567891,1234567891};
      QStringList list = convertToQStringList(vec);
      QStringList expectedList{"-1234567891","1234567891"};
      EXPECT_EQ(list,expectedList);
   }
}

TEST(Qt_UtilsTest, testDoubleVecToStringList)
{
   using namespace qtutils;
   {
      QVector<double> vec{};
      QStringList list = convertToQStringList(vec);
      EXPECT_TRUE(list.size() == 0);
   }
   {
      QVector<double> vec{-999999999999999999999999999999999.0};
      QStringList list = convertToQStringList(vec);
      QStringList expectedList{"-1e+33"};
      EXPECT_EQ(list,expectedList);
   }
   {
      QVector<double> vec{-999999999999999999999999999999999.0,12345678910111213.0};
      QStringList list = convertToQStringList(vec);
      QStringList expectedList{"-1e+33","1.23456789101e+16"};
      EXPECT_EQ(list,expectedList);
   }
}

TEST(Qt_UtilsTest, testStringListVecToStringList)
{
   QString listSeparator = "][";

   using namespace qtutils;
   {
      QVector<QStringList> vec{};
      QStringList list = mergeQStringLists(vec,listSeparator);
      EXPECT_TRUE(list.size() == 0);
   }
   {
      QVector<QStringList> vec{{"\n/n\t"}};
      QStringList list = mergeQStringLists(vec,listSeparator);
      QStringList expectedList{"\n/n\t"};
      EXPECT_EQ(list,expectedList);
   }
   {
      QVector<QStringList> vec{{"\n/n\t","\n/n\t"},{"\n/n\t","\n/n\t"}};
      QStringList list = mergeQStringLists(vec,listSeparator);
      QStringList expectedList{"\n/n\t][\n/n\t","\n/n\t][\n/n\t"};
      EXPECT_EQ(list,expectedList);
   }
   //QStringList convertToQStringList(const QVector<QStringList>& vec);
}
