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

   scenario.setIsDoeOptionSelected(0,true);
   scenario.setIsQcDoeOptionSelected(0,true);
   scenario.predictionTargetManager().addSurfaceTarget(196750,606750,"Surface1",{"Temperature"});

   targetQCdataCreator::readTargetQCs(scenario);
   const QVector<TargetQC> targetQCs = scenario.targetQCs();

   EXPECT_EQ(targetQCs[0].identifier(),"XYPointLayerTopSurface_Temperature_196750_606750_Layer1_0.0_1.0_1.0");
}

TEST( TestTargetQCDataCreator, TestRemoveObservabledFailedRun )
{
   QVector<QVector<double>> targetDataOrig{{1,2,10},{3,4,12},{5,6,14},{7,8,16}};

   QVector<QVector<double>> targetData = targetDataOrig;
   QVector<int> simulationStates{1,0,1};
   targetQCdataCreator::removeObservablesFailedSimulations(targetData,simulationStates);
   QVector<QVector<double>> dataExpected{{1,10},{3,12},{5,14},{7,16}};
   EXPECT_EQ(targetData,dataExpected);

   simulationStates = {0,0,0};
   targetData = targetDataOrig;
   targetQCdataCreator::removeObservablesFailedSimulations(targetData,simulationStates);
   dataExpected = {{},{},{},{}};
   EXPECT_EQ(targetData,dataExpected);

   //Should throw for invalid input:
   simulationStates = {1,0};
   targetData = targetDataOrig;
   EXPECT_THROW(targetQCdataCreator::removeObservablesFailedSimulations(targetData,simulationStates), std::runtime_error);

   simulationStates = {1,0,1};
   targetData = {{1,2,10},{3,4,12},{5,6,14},{7,8,16,0}};
   EXPECT_THROW(targetQCdataCreator::removeObservablesFailedSimulations(targetData,simulationStates), std::runtime_error);
}
