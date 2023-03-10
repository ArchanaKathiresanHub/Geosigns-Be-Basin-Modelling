#include "model/LithologyScenario.h"

#include "expectFileEq.h"
#include "stubProjectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "stubmapreader.h"

#include <gtest/gtest.h>

TEST(LithologyScenarioTest, testWriteRead)
{
  casaWizard::sac::lithology::LithologyScenario writeScenario{new casaWizard::StubProjectReader()};

  writeScenario.setT2zReferenceSurface(321);
  writeScenario.setT2zNumberCPUs(12);
  writeScenario.setT2zSubSampling(3);
  writeScenario.mapsManager().setSmartGridding(false);

  casaWizard::sac::lithology::LithofractionManager& lithofractionManagerWrite = writeScenario.lithofractionManager();

  lithofractionManagerWrite.addLithofraction("Litho1");
  lithofractionManagerWrite.addLithofraction("Litho2");

  lithofractionManagerWrite.setLithofractionFirstComponent(0, 1);
  lithofractionManagerWrite.setLithofractionFirstMinPercentage(0, 2);
  lithofractionManagerWrite.setLithofractionFirstMaxPercentage(0, 3);
  lithofractionManagerWrite.setLithofractionSecondComponent(0, 2);
  lithofractionManagerWrite.setLithofractionSecondMinFraction(0, 0.5);
  lithofractionManagerWrite.setLithofractionSecondMaxFraction(0, 0.6);

  casaWizard::sac::lithology::OptimizedLithofraction optim(10, 11, 12, 13);
  lithofractionManagerWrite.addOptimizedLithofraction(optim);

  casaWizard::sac::WellTrajectoryManager& wellTrajectoryManagerWrite = writeScenario.wellTrajectoryManager();
  wellTrajectoryManagerWrite.addWellTrajectory(15, "TemperatureUserName");
  wellTrajectoryManagerWrite.addWellTrajectory(16, "TemperatureUserName");
  wellTrajectoryManagerWrite.setTrajectoryData(casaWizard::sac::TrajectoryType::Original1D, 0, {1, 2, 3}, {4, 5, 6});
  wellTrajectoryManagerWrite.setTrajectoryData(casaWizard::sac::TrajectoryType::Optimized1D, 0, {1, 2, 3},  {7, 8, 9});

  casaWizard::ScenarioWriter writer{"scenario.dat"};
  writeScenario.writeToFile(writer);

  writer.close();

  casaWizard::sac::lithology::LithologyScenario readScenario{new casaWizard::StubProjectReader()};
  casaWizard::ScenarioReader reader{"scenario.dat"};
  readScenario.readFromFile(reader);

  EXPECT_EQ(writeScenario.t2zLastSurface(), readScenario.t2zLastSurface());
  EXPECT_EQ(writeScenario.t2zLastSurface(), 10); // Since stub reader returns 10 voor lowest layer with TWTT
  EXPECT_EQ(writeScenario.t2zReferenceSurface(), readScenario.t2zReferenceSurface());
  EXPECT_EQ(writeScenario.t2zNumberCPUs(), readScenario.t2zNumberCPUs());
  EXPECT_EQ(writeScenario.t2zNumberCPUs(), 12);
  EXPECT_EQ(writeScenario.t2zSubSampling(), 3);
  EXPECT_EQ(writeScenario.t2zSubSampling(), readScenario.t2zSubSampling());
  EXPECT_EQ(writeScenario.mapsManager().smartGridding(), readScenario.mapsManager().smartGridding());

  EXPECT_FALSE(writeScenario.t2zRunOnOriginalProject());
  writeScenario.setT2zRunOnOriginalProject(true);
  EXPECT_TRUE(writeScenario.t2zRunOnOriginalProject());


  const casaWizard::sac::lithology::LithofractionManager& lithofractionManagerRead = readScenario.lithofractionManager();

  const QVector<casaWizard::sac::lithology::Lithofraction>& lithofractions = lithofractionManagerRead.lithofractions();
  ASSERT_EQ(lithofractions.size(), 2);
  EXPECT_EQ(lithofractions[0].layerName().toStdString(), "Litho1");
  EXPECT_EQ(lithofractions[0].firstComponent(), 1);
  EXPECT_EQ(lithofractions[0].secondComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].minPercentageFirstComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxPercentageFirstComponent(), 3);
  EXPECT_DOUBLE_EQ(lithofractions[0].minFractionSecondComponent(), 0.5);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxFractionSecondComponent(), 0.6);
  EXPECT_EQ(lithofractions[1].layerName().toStdString(), "Litho2");

  const QVector<casaWizard::sac::lithology::OptimizedLithofraction>& optimized = lithofractionManagerRead.optimizedLithofractions();
  ASSERT_EQ(optimized.size(), 1);
  EXPECT_EQ(optimized[0].wellId(), 10);
  EXPECT_EQ(optimized[0].lithofractionId(), 11);
  EXPECT_DOUBLE_EQ(optimized[0].optimizedPercentageFirstComponent(), 12);
  EXPECT_DOUBLE_EQ(optimized[0].optimizedFractionSecondComponent(), 13);

  const casaWizard::sac::WellTrajectoryManager& wellTrajectoryManagerRead = readScenario.wellTrajectoryManager();

  const QVector<QVector<casaWizard::sac::WellTrajectory>> trajectoriesInWell = wellTrajectoryManagerRead.trajectoriesInWell({15}, {"TemperatureUserName"});
  const casaWizard::sac::WellTrajectory baseRunWell = trajectoriesInWell[casaWizard::sac::TrajectoryType::Original1D][0];
  const casaWizard::sac::WellTrajectory bestMatchWell = trajectoriesInWell[casaWizard::sac::TrajectoryType::Optimized1D][0];

  EXPECT_EQ(baseRunWell.propertyUserName().toStdString(), "TemperatureUserName");

  EXPECT_EQ(baseRunWell.wellIndex(), 15);
  EXPECT_EQ(baseRunWell.trajectoryIndex(), 0);
  QVector<double> depth = {1, 2, 3};
  QVector<double> baseRun = {4, 5, 6};
  QVector<double> bestMatch =  {7, 8, 9};
  for (int i=0; i<3; ++i)
  {
    EXPECT_EQ(baseRunWell.depth()[i], depth[i]);
    EXPECT_EQ(baseRunWell.value()[i], baseRun[i]);
    EXPECT_EQ(bestMatchWell.depth()[i], depth[i]);
    EXPECT_EQ(bestMatchWell.value()[i], bestMatch[i]);
  }

  casaWizard::ScenarioWriter writerLoaded{"scenarioAgain.dat"};
  readScenario.writeToFile(writerLoaded);
  writerLoaded.close();
  expectFileEq("scenario.dat", "scenarioAgain.dat");

  readScenario.clear();
  EXPECT_EQ(readScenario.t2zReferenceSurface(), 0);
  EXPECT_EQ(readScenario.t2zSubSampling(), 1);
  EXPECT_FALSE(readScenario.t2zRunOnOriginalProject());
  EXPECT_EQ(readScenario.t2zNumberCPUs(), 1);
}

TEST(LithologyScenarioTest, testWellPrepToSAC)
{
  // Given
  casaWizard::sac::lithology::LithologyScenario scenario{new casaWizard::StubProjectReader()};

  scenario.calibrationTargetManagerWellPrep().addWell("WellName", 1.0, 2.0);
  scenario.setWorkingDirectory(".");
  scenario.setProject3dFileNameAndLoadFile("projStub.project3d");
  scenario.calibrationTargetManagerWellPrep().addCalibrationTarget("TwoWayTime", "TwoWayTime", 0, 10.0, 1000.0);
  scenario.calibrationTargetManagerWellPrep().addCalibrationTarget("TwoWayTime", "TwoWayTime", 0, 30.0, 2000.0);

  // When
  scenario.wellPrepToSAC();

  // Then
  const QVector<const casaWizard::Well*> wells = scenario.calibrationTargetManagerWellPrep().wells();
  const QVector<casaWizard::ObjectiveFunctionValue>& values = scenario.objectiveFunctionManager().values();
  const QVector<QVector<casaWizard::sac::WellTrajectory>> trajectories = scenario.wellTrajectoryManager().trajectories();

  EXPECT_EQ(wells.size(), 1);
  EXPECT_EQ(values.size(), 1);
  EXPECT_EQ(trajectories.size(), 4);
  EXPECT_EQ(trajectories[0].size(), 1);
  EXPECT_EQ(trajectories[1].size(), 1);
  EXPECT_EQ(trajectories[2].size(), 1);
  EXPECT_EQ(trajectories[3].size(), 1);
}

void createScenarioWithOptimizedLithofractions(casaWizard::sac::lithology::LithologyScenario& scenario)
{
  scenario.calibrationTargetManager().addWell("ActiveWell1", 1000, 1000);
  scenario.calibrationTargetManager().addWell("ActiveWell2", 3000, 3000);
  scenario.calibrationTargetManager().addWell("Non-ActiveWell", 2000, 2000);
  scenario.calibrationTargetManager().addWell("Disabled-Well", 2001, 2000);
  scenario.calibrationTargetManager().setWellIsActive(false, 2);
  scenario.calibrationTargetManager().setWellHasActiveProperties(false, 3);

  casaWizard::sac::lithology::OptimizedLithofraction optimizedLithofractionWell1(0, 0, 60, 0.5);
  casaWizard::sac::lithology::OptimizedLithofraction optimizedLithofractionWell2(1, 0, 50, 0.2);
  casaWizard::sac::lithology::OptimizedLithofraction optimizedLithofractionNonActiveWell(2, 0, 40, 0.5);

  scenario.lithofractionManager().addOptimizedLithofraction(optimizedLithofractionWell1);
  scenario.lithofractionManager().addOptimizedLithofraction(optimizedLithofractionWell2);
  scenario.lithofractionManager().addOptimizedLithofraction(optimizedLithofractionNonActiveWell);
  scenario.lithofractionManager().addLithofraction("layer");
}

TEST(LithologyScenarioTest, testGetLithopercentagesOfClosestWell)
{
  // Given
  casaWizard::sac::lithology::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  createScenarioWithOptimizedLithofractions(scenario);

  // When
  int closestWellID = -1;
  std::vector<double> lithoPercentagesOfClosestWell = scenario.getLithopercentagesOfClosestWell(2.2, 2.2, "layer", closestWellID);

  // Then
  EXPECT_EQ(closestWellID, 1);
  EXPECT_DOUBLE_EQ(lithoPercentagesOfClosestWell[0], 50);
  EXPECT_DOUBLE_EQ(lithoPercentagesOfClosestWell[1], 10);
  EXPECT_DOUBLE_EQ(lithoPercentagesOfClosestWell[2], 40);
}

TEST(LithologyScenarioTest, testGetLithopercentagesOfClosestWellNoLithoFractionInLayer)
{
  // Given
  casaWizard::sac::lithology::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  createScenarioWithOptimizedLithofractions(scenario);

  // When
  int closestWellID = -1;
  std::vector<double> lithoPercentagesOfClosestWell = scenario.getLithopercentagesOfClosestWell(1.9, 1.9, "non-existent layer", closestWellID);

  // Then
  EXPECT_EQ(closestWellID, -1);
  EXPECT_TRUE(lithoPercentagesOfClosestWell.empty());
}

TEST(LithologyScenarioTest, getSurfaceValuesForWell)
{
   casaWizard::sac::lithology::LithologyScenario scenario(new casaWizard::StubProjectReader());

   casaWizard::Well well;
   well.setX(100);
   well.setY(200);
   casaWizard::StubMapReader mapReader;

   // In the stub projectreader, the second depthgrid will return an empty string
   // meaning there is no depth map available. Still we would like to continue
   // Without the getSurfaceValuesForWell function throwing. It should just
   // not return any surface values.
   EXPECT_NO_THROW(scenario.getSurfaceValuesForWell(well, mapReader));
   EXPECT_EQ(scenario.getSurfaceValuesForWell(well, mapReader).size(), 0);
}

TEST(LithologyScenarioTest, testSetCalibrationTargetsBasedOnObjectiveFunctions)
{
   // Given
   casaWizard::sac::lithology::LithologyScenario scenario{new casaWizard::StubProjectReader()};

   const int activeWellID = scenario.calibrationTargetManager().addWell("ActiveWell", 1000, 1000);
   const int multiWellID = scenario.calibrationTargetManager().addWell("MultiWell", 20000, 2000);
   const int deactivatedWellID = scenario.calibrationTargetManager().addWell("DeactivatedWell", 3000, 3000);

   scenario.calibrationTargetManager().addCalibrationTarget("Target 1", "Enable",  activeWellID,      1.0, 2.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Target 2", "Enable",  multiWellID,       3.0, 4.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Target 2", "Disable", multiWellID,       5.0, 6.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Target 3", "Disable", deactivatedWellID, 7.0, 8.0);

   const QStringList vars{"Enable", "Disable"};
   QMap<QString, QString> mapping;
   mapping["Enable"] = "enable";
   mapping["Disable"] = "disable";

   scenario.objectiveFunctionManager().setVariables(vars, mapping);

    // When
   scenario.objectiveFunctionManager().setEnabledState(false, 1);
   scenario.setCalibrationTargetsBasedOnObjectiveFunctions();

   // Then
   EXPECT_TRUE(scenario.calibrationTargetManager().well(activeWellID).hasActiveProperties());
   EXPECT_TRUE(scenario.calibrationTargetManager().well(multiWellID).hasActiveProperties());
   EXPECT_FALSE(scenario.calibrationTargetManager().well(deactivatedWellID).hasActiveProperties());
}
