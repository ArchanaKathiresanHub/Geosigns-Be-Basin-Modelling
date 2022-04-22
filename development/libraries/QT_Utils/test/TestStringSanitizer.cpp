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

TEST( StringSanitizerTest, testEscapeSpecialCharacters)
{
   QString stringWithSpecialCharacters = "\\\\'\"\?\a\b\f\n\r\t\vjh";

   QString escapedString = qtutils::escapeSpecialCharacters(stringWithSpecialCharacters);
   EXPECT_EQ(escapedString.toStdString(), "\\\\'\"\?\\a\\b\\f\\n\\r\\t\\vjh");

   //Test that the escapes are not again escaped.
   QString escapedString2 = qtutils::escapeSpecialCharacters(escapedString);
   EXPECT_EQ(escapedString, escapedString2);
}
