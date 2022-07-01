#include "stubProjectReader.h"

#include "model/input/targetQCdataCreator.h"
#include "model/uaScenario.h"
#include "model/targetQC.h"

#include <gtest/gtest.h>

using namespace casaWizard::ua;

TEST( TestTargetQCDataCreator, TestCreateFromFile )
{
   casaWizard::ua::UAScenario scenario{new casaWizard::StubProjectReader()};
   scenario.setWorkingDirectory("./testTargetQCDataCreatorInput/");

   scenario.predictionTargetManager().addSurfaceTarget(196750,606750,"Surface1",{"Temperature"});

   targetQCdataCreator::readTargetQCs(scenario);
   const QVector<TargetQC> targetQCs = scenario.targetQCs();

   EXPECT_EQ(targetQCs[0].identifier(),"XYPointLayerTopSurface_Temperature_196750_606750_Layer1_0.0_1.0_1.0");
}
