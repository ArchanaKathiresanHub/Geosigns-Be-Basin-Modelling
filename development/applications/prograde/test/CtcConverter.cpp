//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

#include "CtcModelConverter.h"


//cmbAPI
#include "CtcManager.h"


using namespace mbapi;

TEST(CtcModelConverter, upgrade)
{
   Prograde::CtcModelConverter modelConverter;
   EXPECT_EQ("Active Rifting", modelConverter.upgradeTectonicFlag(500.0, 125.0)); 
   EXPECT_EQ("Passive Margin", modelConverter.upgradeTectonicFlag(500.0, 725.0));
   EXPECT_EQ("Flexural Basin", modelConverter.upgradeTectonicFlag(0.0, 725.0));
}