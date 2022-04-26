//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "StubUAScenario.h"

#include "model/calibrationTargetManager.h"
#include "model/PredictionTargetsFromWellsCreator.h"
#include "model/predictionTargetManager.h"
#include "model/TargetInputFromWellsInfo.h"

#include <gtest/gtest.h>

using namespace casaWizard;
using namespace ua;

class PredictionTargetFromWellsCreatorTest : public ::testing::Test
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

   TargetInputFromWellsInfo& inputInfo()
   {
      return m_inputInfo;
   }

   void SetUp() override
   {
      CalibrationTargetManager& calibrationTargetManager = m_scenario.calibrationTargetManager();
      calibrationTargetManager.addWell("w1",1,2);
      calibrationTargetManager.addWell("w2",3,4);

      m_inputInfo.wellSelectionStates = {true, false};
      m_inputInfo.surfaceSelectionStates = QVector<bool>(predictionTargetManager().validSurfaceNames().size(),false);
      m_inputInfo.vreTargetsSelected = true;
      m_inputInfo.temperatureTargetsSelected = true;
   }

private:
   TargetInputFromWellsInfo m_inputInfo;
   StubUAScenario m_scenario;
};

TEST_F( PredictionTargetFromWellsCreatorTest, testSurfaceInput )
{
   *inputInfo().surfaceSelectionStates.begin() = true;
   PredictionTargetsFromWellsCreator::createTargetsFromWellData(inputInfo(),
                                                                calibrationTargetManager(),
                                                                predictionTargetManager());
   ASSERT_EQ(predictionTargetManager().amountAtAge0(),1);
   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   EXPECT_EQ(predictionTargets.at(0)->variable().toStdString(),"\"Layer\"");
}

TEST_F( PredictionTargetFromWellsCreatorTest, locationNaming )
{
   PredictionTargetsFromWellsCreator::createTargetsFromWellData(inputInfo(),
                                                                calibrationTargetManager(),
                                                                predictionTargetManager());
   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();

   if (predictionTargets.size() > 0 && calibrationTargetManager().wells().size() > 0)
   {
      EXPECT_EQ(predictionTargets.at(0)->locationName(),calibrationTargetManager().well(0).name());
   }
}

TEST_F( PredictionTargetFromWellsCreatorTest, testDepthInput )
{
   inputInfo().depthInput = "10,, 20 , 10df, 40\n, lsdf, 40\n asdposak";

   PredictionTargetsFromWellsCreator::createTargetsFromWellData(inputInfo(),
                                                                calibrationTargetManager(),
                                                                predictionTargetManager());

   ASSERT_EQ(predictionTargetManager().amountAtAge0(),3);
   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   EXPECT_EQ(predictionTargets.at(0)->z(),10);
   EXPECT_EQ(predictionTargets.at(1)->z(),20);
   EXPECT_EQ(predictionTargets.at(2)->z(),40);
}

#ifndef NDEBUG
TEST_F( PredictionTargetFromWellsCreatorTest, testWellSelectionInfoSizeMismatch )
{
   //Mismatch in inputInfo and well data
   calibrationTargetManager().addWell("w3",5,6);
   EXPECT_EXIT(PredictionTargetsFromWellsCreator::createTargetsFromWellData(inputInfo(),
                                                                            calibrationTargetManager(),
                                                                            predictionTargetManager()), ::testing::KilledBySignal(SIGABRT), "");
}

TEST_F( PredictionTargetFromWellsCreatorTest, testSurfaceSelectionInfoSizeMismatch )
{
   //Mismatch in inputInfo and well data
   inputInfo().surfaceSelectionStates.push_back(false);
   EXPECT_EXIT(PredictionTargetsFromWellsCreator::createTargetsFromWellData(inputInfo(),
                                                                            calibrationTargetManager(),
                                                                            predictionTargetManager()), ::testing::KilledBySignal(SIGABRT), "");
}
#endif
