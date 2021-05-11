#include "model/sacScenario.h"

#include "expectFileEq.h"
#include "stubProjectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include <gtest/gtest.h>

TEST(SACScenarioTest, testWriteRead)
{
  casaWizard::sac::SACScenario writeScenario{new casaWizard::StubProjectReader()};

  writeScenario.setT2zReferenceSurface(321);
  writeScenario.setT2zNumberCPUs(12);
  writeScenario.setT2zSubSampling(3);

  casaWizard::sac::LithofractionManager& lithofractionManagerWrite = writeScenario.lithofractionManager();

  lithofractionManagerWrite.addLithofraction("Litho1");
  lithofractionManagerWrite.addLithofraction("Litho2");

  lithofractionManagerWrite.setLithofractionFirstComponent(0, 1);
  lithofractionManagerWrite.setLithofractionFirstMinPercentage(0, 2);
  lithofractionManagerWrite.setLithofractionFirstMaxPercentage(0, 3);
  lithofractionManagerWrite.setLithofractionSecondComponent(0, 2);
  lithofractionManagerWrite.setLithofractionSecondMinFraction(0, 0.5);
  lithofractionManagerWrite.setLithofractionSecondMaxFraction(0, 0.6);

  casaWizard::sac::OptimizedLithofraction optim(10, 11, 12, 13);
  lithofractionManagerWrite.addOptimizedLithofraction(optim);

  casaWizard::sac::WellTrajectoryManager& wellTrajectoryManagerWrite = writeScenario.wellTrajectoryManager();
  wellTrajectoryManagerWrite.addWellTrajectory(15, "TemperatureUserName");
  wellTrajectoryManagerWrite.addWellTrajectory(16, "TemperatureUserName");
  wellTrajectoryManagerWrite.setTrajectoryData(casaWizard::sac::TrajectoryType::Original1D, 0, {1, 2, 3}, {4, 5, 6});
  wellTrajectoryManagerWrite.setTrajectoryData(casaWizard::sac::TrajectoryType::Optimized1D, 0, {1, 2, 3},  {7, 8, 9});

  casaWizard::ScenarioWriter writer{"scenario.dat"};
  writeScenario.writeToFile(writer);

  writer.close();

  casaWizard::sac::SACScenario readScenario{new casaWizard::StubProjectReader()};
  casaWizard::ScenarioReader reader{"scenario.dat"};
  readScenario.readFromFile(reader);

  EXPECT_EQ(writeScenario.t2zLastSurface(), readScenario.t2zLastSurface());
  EXPECT_EQ(writeScenario.t2zLastSurface(), 10); // Since stub reader returns 10 voor lowest layer with TWTT
  EXPECT_EQ(writeScenario.t2zReferenceSurface(), readScenario.t2zReferenceSurface());
  EXPECT_EQ(writeScenario.t2zNumberCPUs(), readScenario.t2zNumberCPUs());
  EXPECT_EQ(writeScenario.t2zNumberCPUs(), 12);
  EXPECT_EQ(writeScenario.t2zSubSampling(), 3);
  EXPECT_EQ(writeScenario.t2zSubSampling(), readScenario.t2zSubSampling());

  EXPECT_FALSE(writeScenario.t2zRunOnOriginalProject());
  writeScenario.setT2zRunOnOriginalProject(true);
  EXPECT_TRUE(writeScenario.t2zRunOnOriginalProject());


  const casaWizard::sac::LithofractionManager& lithofractionManagerRead = readScenario.lithofractionManager();

  const QVector<casaWizard::sac::Lithofraction>& lithofractions = lithofractionManagerRead.lithofractions();
  ASSERT_EQ(lithofractions.size(), 2);
  EXPECT_EQ(lithofractions[0].layerName().toStdString(), "Litho1");
  EXPECT_EQ(lithofractions[0].firstComponent(), 1);
  EXPECT_EQ(lithofractions[0].secondComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].minPercentageFirstComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxPercentageFirstComponent(), 3);
  EXPECT_DOUBLE_EQ(lithofractions[0].minFractionSecondComponent(), 0.5);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxFractionSecondComponent(), 0.6);
  EXPECT_EQ(lithofractions[1].layerName().toStdString(), "Litho2");

  const QVector<casaWizard::sac::OptimizedLithofraction>& optimized = lithofractionManagerRead.optimizedLithofractions();
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
