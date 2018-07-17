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

#include "BiodegradeConverter.h"


//cmbAPI
#include "BiodegradeManager.h"


using namespace mbapi;

TEST(BiodegradeConverter, upgrade)
{
   Prograde::BiodegradeConverter modelConverter;
   
   // if MaxBioTemp > 100.0
   double BioConsValue = 80.0; // Default value
   double BioConsFromP3dFile = 110.0;
   modelConverter.upgradeBioConstants("MaxBioTemp", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // if MaxBioTemp < 0.0
   BioConsValue = 80.0;
   BioConsFromP3dFile = -80.0;
   modelConverter.upgradeBioConstants("MaxBioTemp", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // if TempConstant > 100.0
   BioConsValue = 80.0;
   BioConsFromP3dFile = 110.0;
   modelConverter.upgradeBioConstants("TempConstant", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // if TempConstant < 0.0
   BioConsValue = 80.0;
   BioConsFromP3dFile = -80.0;
   modelConverter.upgradeBioConstants("TempConstant", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // if TimeConstant > 100.0
   BioConsValue = 80.0;
   BioConsFromP3dFile = 110.0;
   modelConverter.upgradeBioConstants("TimeConstant", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // if TimeConstant < 0.0
   BioConsValue = 80.0;
   BioConsFromP3dFile = -80.0;
   modelConverter.upgradeBioConstants("TimeConstant", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // if BioRate > 10000.0
   BioConsValue = 1000.0;
   BioConsFromP3dFile = 11000.0;
   modelConverter.upgradeBioConstants("BioRate", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // if BioRate < 0.0
   BioConsValue = 1000.0;
   BioConsFromP3dFile = -1000.0;
   modelConverter.upgradeBioConstants("BioRate", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);   
   
   // Rest of the biodegradation constant value != Default value
   BioConsValue = 0.0008;
   BioConsFromP3dFile = 0.0009;
   modelConverter.upgradeBioConstants("C1_BioFactor", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);
   
   // Rest of the biodegradation constant value == Default value
   BioConsValue = 0.0008;
   BioConsFromP3dFile = 0.0008;
   modelConverter.upgradeBioConstants("C1_BioFactor", BioConsValue, BioConsFromP3dFile);
   ASSERT_NEAR(BioConsValue, BioConsFromP3dFile, 1e-6);   
   
}