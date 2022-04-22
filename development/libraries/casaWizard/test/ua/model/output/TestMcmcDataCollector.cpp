//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/doeOption.h"
#include "model/input/projectReader.h"
#include "model/output/McmcOutputDataCollector.h"
#include "model/output/McmcTargetExportData.h"
#include "model/uaScenario.h"
#include "StubUAScenario.h"

#include <gtest/gtest.h>
#include <QDir>

using namespace casaWizard;
using namespace ua;

TEST(McmcDataCollector, testCollecting )
{
   StubUAScenario scenario;
   McmcTargetExportData exportData = McmcOutputDataCollector::collectMcmcOutputData(scenario);

   //Empty scenario: no export data
   EXPECT_TRUE(exportData.allPropNames.size()==0);
   EXPECT_TRUE(exportData.targetData.size()==0);

   PredictionTargetManager& ptManager = scenario.predictionTargetManager();
   ptManager.addDepthTarget(4.0, 5.0, 6.0, {"VRe"}, 0.0);

   MonteCarloDataManager& mcManager = scenario.monteCarloDataManager();
   mcManager.setPredictionTargetMatrix({{55, 56, 57}});

   exportData = McmcOutputDataCollector::collectMcmcOutputData(scenario);

   //Expect VRe to be there
   EXPECT_TRUE(exportData.allPropNames.find("VRe") != exportData.allPropNames.end());

   mcManager.setRmse({1, 2});
   mcManager.setCalibrationTargetMatrix({{11, 12, 13}, {21, 22, 23}});
   mcManager.setInfluentialParameterMatrix({{33, 34, 35}, {43, 44, 35}});
   mcManager.setPredictionTargetMatrix({{55, 56, 57}, {65, 66, 67}});

   ptManager.addDepthTarget(4.0, 5.0, 6.0,{"Temperature"},0.0);
   exportData = McmcOutputDataCollector::collectMcmcOutputData(scenario);

   //Expect Temperature to be there
   EXPECT_TRUE(exportData.allPropNames.find("Temperature") != exportData.allPropNames.end());

   //Data is merged because of same location and age.
   EXPECT_TRUE(exportData.targetData.size() == 1);

   //DoE type and order is used to determine if the base case is run.
   scenario.setIsDoeOptionSelected(4,true);
   EXPECT_EQ(scenario.doeOptions()[4]->name(),"FullFactorial");

   scenario.setWorkingDirectory(QDir::currentPath());
   exportData = McmcOutputDataCollector::collectMcmcOutputData(scenario);

   exportData = McmcOutputDataCollector::collectMcmcOutputData(scenario);
   const TargetDataSingleProperty& prop = exportData.targetData.first().targetOutputs["Temperature"];

   //Values read from obsevables file
   EXPECT_EQ(prop.baseProxy, -1e60);
   EXPECT_EQ(prop.baseSim, -1e60);

   QVector<TargetQC> targetQCs;
   TargetQC t1(0.0,"Temperature","",false,0.0, 0.0, 0,0,0, {65},{66});
   TargetQC t2(1.0,"VRe","",false,0.0, 0.0, 0, 0, 0, {55},{56});
   targetQCs.push_back(t1);
   targetQCs.push_back(t2);
   scenario.setTargetQCs(targetQCs);

   {
      exportData = McmcOutputDataCollector::collectMcmcOutputData(scenario);
      const TargetDataSingleProperty& prop = exportData.targetData.first().targetOutputs["Temperature"];

      //Values read from target Qc's (and from monte carlo data):
      EXPECT_EQ(prop.baseProxy, 56.0);
      EXPECT_EQ(prop.baseSim, 55.0);
      EXPECT_EQ(prop.optimalProxy, 65.0);
      EXPECT_EQ(prop.p10, 65.0);
      EXPECT_EQ(prop.p50, 66.0);
      EXPECT_EQ(prop.p90, 67.0);
      EXPECT_TRUE(isnan(prop.optimalSim));
   }

   {
      scenario.setIsDoeOptionSelected(2,true);
      EXPECT_EQ(scenario.doeOptions()[2]->name(),"PlackettBurman");
      exportData = McmcOutputDataCollector::collectMcmcOutputData(scenario);
      const TargetDataSingleProperty& prop = exportData.targetData.first().targetOutputs["temperature"];

      //If PlackettBurman was run, the base case is not the first case and is not guaranteed to be there, its not read.
      //(if however, Tornado of Box-Behnken was run before PlackettBurman, we do read the base case as the first case.)
      EXPECT_TRUE(isnan(prop.baseProxy));
      EXPECT_TRUE(isnan(prop.baseSim));
   }
}
