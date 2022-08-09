#include "StubUAScenario.h"

#include "model/calibrationTargetManager.h"
#include "model/predictionTargetManager.h"
#include "model/PredictionTargetCreator.h"
#include "model/TargetInputInfoFromWells.h"

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

   TargetInputInfoFromWells& wellInfo(){
      return m_wellInputInfo;
   }

   void SetUp() override
   {
      CalibrationTargetManager& calibrationTargetManager = m_scenario.calibrationTargetManager();
      calibrationTargetManager.addWell("w1",1,2);
      calibrationTargetManager.addWell("w2",3,4);

      m_wellInputInfo.setWellSelectionStates({true, false});
      m_wellInputInfo.setSurfaceSelectionStates(QVector<bool>(predictionTargetManager().validSurfaceNames().size(),false));
      m_wellInputInfo.setTemperatureTargetsSelected(true);
      m_wellInputInfo.setVreTargetsSelected(true);
      m_wellInputInfo.setDepthInput("");
   }

private:
   StubUAScenario m_scenario;
   TargetInputInfoFromWells m_wellInputInfo = TargetInputInfoFromWells(calibrationTargetManager(),{},{});
};


#ifndef NDEBUG
TEST_F( PredictionTargetCreatorTest, testWellSelectionInfoSizeMismatch )
{
   //Mismatch in inputInfo and well data
   calibrationTargetManager().addWell("w3",5,6);

   PredictionTargetCreator creator(wellInfo(), predictionTargetManager());
   EXPECT_EXIT(creator.createTargets();, ::testing::KilledBySignal(SIGABRT), "");
}

TEST_F( PredictionTargetCreatorTest, testSurfaceSelectionInfoSizeMismatch )
{
   //Mismatch in inputInfo and well data
   wellInfo().setWellSelectionStates({true, false,false});

   PredictionTargetCreator creator(wellInfo(), predictionTargetManager());
   EXPECT_EXIT(creator.createTargets();, ::testing::KilledBySignal(SIGABRT), "");
}
#endif
