//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/targetQC.h"

#include <gtest/gtest.h>

using namespace casaWizard::ua;

TEST( TestTargetQC, TestSaveLoad )
{
   const int id = 2;
   const QString& property = "Temperature";
   const QString& name = "testTarget";
   const QString& identifier ="12345";
   const bool calibration = false;
   const double value = 40;
   const double standardDeviation = 20;
   const double R2 = 0.99;
   const double R2Adj = 0.9;
   const double Q = 0.8;
   const QVector<double>& y = {1,2,3};
   const QVector<double>& yProxy  = {2,3,4};
   const double yOptimalSim = 5;

   //Writing a target to a stringlist and constructing a new target from the stringlist should lead to the same target:
   TargetQC targetQC(id,property,name,identifier,calibration,value,standardDeviation,R2,R2Adj,Q,y,yProxy,yOptimalSim);
   const QStringList writeList = targetQC.write();
   const TargetQC readTarget = TargetQC::read(1,writeList);
   EXPECT_EQ(targetQC.write(),readTarget.write());
}

TEST( TestTargetQC, TestLoadOldFormat )
{
   const QStringList readList{"2",
                        "Temperature",
                        "testTarget",
                        "0",
                        "40",
                        "20",
                        "0.99",
                        "0.9",
                        "0.8",
                        "1.000000><2.000000><3.000000",
                        "2.000000><3.000000><4.000000"};

   const TargetQC readTarget = TargetQC::read(0,readList);
   EXPECT_EQ(readTarget.id(),2);
   EXPECT_EQ(readTarget.property(),"Temperature");
   EXPECT_EQ(readTarget.name(),"testTarget");
   EXPECT_EQ(readTarget.identifier(),"");
   EXPECT_EQ(readTarget.calibration(),false);
   EXPECT_EQ(readTarget.value(),40);
   EXPECT_EQ(readTarget.standardDeviation(),20);
   EXPECT_EQ(readTarget.R2(),0.99);
   EXPECT_EQ(readTarget.R2Adj(),0.9);
   EXPECT_EQ(readTarget.Q2(),0.8);
   EXPECT_EQ(readTarget.y()[1],2);
   EXPECT_EQ(readTarget.yProxy()[1],3);
}
