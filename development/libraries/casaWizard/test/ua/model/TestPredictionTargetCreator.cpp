//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "StubUAScenario.h"
#include "StubInputInfo.h"

#include "model/calibrationTargetManager.h"
#include "model/predictionTargetManager.h"
#include "model/PredictionTargetCreator.h"

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

   stubTargetInputInfo& inputInfo(){
      return m_inputInfo;
   }

   void SetUp() override
   {
      m_inputInfo.setSurfaceSelectionStates(QVector<bool>(predictionTargetManager().validSurfaceNames().size(),false));
      m_inputInfo.setTemperatureTargetsSelected(true);
      m_inputInfo.setVreTargetsSelected(true);
      m_inputInfo.setDepthInput("");
   }

private:
   StubUAScenario m_scenario;
   stubTargetInputInfo m_inputInfo = stubTargetInputInfo (predictionTargetManager());
};


TEST_F( PredictionTargetCreatorTest, testSurfaceInput )
{
   QVector<bool> surfaces(predictionTargetManager().validSurfaceNames().size(),false);
   surfaces.replace(0, true);
   inputInfo().setSurfaceSelectionStates(surfaces);

   PredictionTargetCreator creator(inputInfo(), predictionTargetManager());
   creator.createTargets();

   ASSERT_EQ(predictionTargetManager().amountAtAge0(), 4); //stub creates 4 targetlocations
   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   EXPECT_EQ(predictionTargets.at(0)->variable().toStdString(),"\"Layer1\"");
}

TEST_F( PredictionTargetCreatorTest, locationNaming )
{
   PredictionTargetCreator creator(inputInfo(), predictionTargetManager());
   creator.createTargets();

   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   if (predictionTargets.size() > 0 && inputInfo().getTargetLocations().size() > 0)
   {
      EXPECT_EQ(predictionTargets.at(0)->locationName(), inputInfo().getTargetLocations()[0].name);
   }
}

TEST_F( PredictionTargetCreatorTest, testDepthInput )
{
   inputInfo().setDepthInput("10,, 20 , 10df, 40\n, lsdf, 40\n asdposak");

   PredictionTargetCreator creator(inputInfo(), predictionTargetManager());
   creator.createTargets();
   ASSERT_EQ(predictionTargetManager().amountAtAge0(),12); //should be 3 successful depthinputs, * 4 locations
   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager().predictionTargets();
   EXPECT_EQ(predictionTargets.at(0)->z(),10);
   EXPECT_EQ(predictionTargets.at(1)->z(),20);
   EXPECT_EQ(predictionTargets.at(2)->z(),40);
}





