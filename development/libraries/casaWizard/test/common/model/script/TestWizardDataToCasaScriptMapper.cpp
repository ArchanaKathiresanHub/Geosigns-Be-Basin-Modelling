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
