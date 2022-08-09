//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "StubUAScenario.h"

#include "model/calibrationTargetManager.h"
#include "model/predictionTargetManager.h"
#include "model/PredictionTargetCreator.h"
#include "model/TargetInputInfoFromASCII.h"

#include <gtest/gtest.h>

using namespace casaWizard;
using namespace ua;

class PredictionTargetCreatorTest : public ::testing::Test
{
public:

   CalibrationTargetManager& calibrationTargetManager()
   {
      return m_scenario.calibrationTargetManager();
   }

   PredictionTargetManager& predictionTargetManager()
   {
      return m_scenario.predictionTargetManager();
   }

   TargetInputInfoFromASCII& asciiInfo(){
      return m_asciiInputInfo;
   }

   void SetUp() override
   {
      m_asciiInputInfo.setFilePath("./importTest.txt");
      m_asciiInputInfo.setSurfaceSelectionStates(QVector<bool>(predictionTargetManager().validSurfaceNames().size(),false));
      m_asciiInputInfo.setTemperatureTargetsSelected(true);
      m_asciiInputInfo.setVreTargetsSelected(true);
      m_asciiInputInfo.setDepthInput("1");
      m_asciiInputInfo.setDefaultName("PWS");
   }

private:
   StubUAScenario m_scenario;
   TargetInputInfoFromASCII m_asciiInputInfo = TargetInputInfoFromASCII("", {});
};

TEST_F( PredictionTargetCreatorTest, testAsciiFileImport )
{
   PredictionTargetCreator creator(asciiInfo(), predictionTargetManager());
   creator.createTargets();

   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   EXPECT_EQ(predictionTargets.at(0)->x(), 100);
   EXPECT_EQ(predictionTargets.at(0)->y(), 100);
   EXPECT_EQ(predictionTargets.at(0)->locationName(), "ImportTest");
}


TEST_F( PredictionTargetCreatorTest, testWrongAsciiFileImport )
{
   //shouldn't find a file, so no entries should be made
   asciiInfo().setFilePath("./non-existant-file.txt");
   PredictionTargetCreator creator(asciiInfo(), predictionTargetManager());
   creator.createTargets();

   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   EXPECT_EQ(predictionTargets.size(), 0);
}

TEST_F( PredictionTargetCreatorTest, testAsciiEdgeCases )
{
   PredictionTargetCreator creator(asciiInfo(), predictionTargetManager());
   creator.createTargets();

   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   EXPECT_EQ(predictionTargets.at(1)->x(), 100.2); //number get rounded when displayed
   EXPECT_EQ(predictionTargets.at(1)->y(), 100.8);
   EXPECT_EQ(predictionTargets.at(2)->x(), 0);
   EXPECT_EQ(predictionTargets.at(2)->y(), 0);
}
