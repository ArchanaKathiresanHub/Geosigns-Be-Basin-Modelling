#include "model/TCHPManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST(TCHPManagerTest, testOptimizedInWell)
{
  casaWizard::sac::thermal::TCHPManager manager;

  const casaWizard::sac::thermal::OptimizedTCHP optim1{1, 1};
  const casaWizard::sac::thermal::OptimizedTCHP optim2{2, 2};
  const casaWizard::sac::thermal::OptimizedTCHP optim3{3, 3};
  manager.addOptimizedTCHP(optim1);
  manager.addOptimizedTCHP(optim2);
  manager.addOptimizedTCHP(optim3);

  casaWizard::sac::thermal::OptimizedTCHP inWell = manager.optimizedInWell(2);
  EXPECT_EQ(inWell.optimizedHeatProduction(), 2);
}

TEST(TCHPManagerTest, testWriteToFile)
{
  casaWizard::sac::thermal::TCHPManager manager;
  manager.setMinTCHP(0.5);
  manager.setMaxTCHP(4.5);

  manager.addOptimizedTCHP(casaWizard::sac::thermal::OptimizedTCHP(0, 0.5));
  manager.addOptimizedTCHP(casaWizard::sac::thermal::OptimizedTCHP(1, 2.5));

  casaWizard::ScenarioWriter writer{"TCHPManagerActual.dat"};
  manager.writeToFile(writer);
  writer.close();

  expectFileEq("TCHPManager.dat", "TCHPManagerActual.dat");
}


TEST(TCHPManagerTest, testReadFromFile)
{
  casaWizard::sac::thermal::TCHPManager manager;
  casaWizard::ScenarioReader reader{"TCHPManager.dat"};

  manager.readFromFile(reader);

  const double min = manager.minTCHP();
  const double max = manager.maxTCHP();
  const QVector<casaWizard::sac::thermal::OptimizedTCHP>& optimized = manager.optimizedTCHPs();

  EXPECT_EQ(min, 0.5);
  EXPECT_EQ(max, 4.5);
  EXPECT_EQ(optimized[0].optimizedHeatProduction(), 0.5);
  EXPECT_EQ(optimized[1].optimizedHeatProduction(), 2.5);
}
