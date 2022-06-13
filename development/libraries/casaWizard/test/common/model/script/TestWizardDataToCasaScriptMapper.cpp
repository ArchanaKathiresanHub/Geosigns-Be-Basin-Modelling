//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/script/WizardDataToCasaScriptMapper.h"

#include <gtest/gtest.h>

using namespace casaWizard::wizardDataToCasaScriptMapper;

TEST(WizardDataToCasaScriptMapper, testMapName)
{
   QString outName = mapName("VRe");
   EXPECT_EQ(outName,"Vr");
}

TEST(WizardDataToCasaScriptMapper, testdoubleToQString)
{
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

TEST(WizardDataToCasaScriptMapper, testdoubleOneDigitToQString)
{
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
