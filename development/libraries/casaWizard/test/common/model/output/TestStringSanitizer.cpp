//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/output/StringSanitizer.h"

#include <gtest/gtest.h>
#include <iostream>

TEST( StringSanitizerTest, testEscapeSpecialCharacters)
{
   //Test that the escapes are not again escaped.
   QString stringWithSpecialCharacters = "\\\\'\"\?\a\b\f\n\r\t\vjh";
   QString escapedString = casaWizard::stringSanitizer::escapeSpecialCharacters(stringWithSpecialCharacters);
   escapedString = casaWizard::stringSanitizer::escapeSpecialCharacters(escapedString);
   escapedString = casaWizard::stringSanitizer::escapeSpecialCharacters(escapedString);

   EXPECT_EQ(escapedString.toStdString(), "\\\\'\"\?\\a\\b\\f\\n\\r\\t\\vjh");
}
