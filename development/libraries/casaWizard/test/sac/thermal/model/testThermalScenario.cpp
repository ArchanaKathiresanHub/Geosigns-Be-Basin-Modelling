#include "model/ThermalScenario.h"

#include "expectFileEq.h"
#include "stubProjectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include <gtest/gtest.h>

TEST(ThermalScenarioTest, testWriteRead)
{
  casaWizard::sac::thermal::ThermalScenario writeScenario{new casaWizard::StubProjectReader()};
  casaWizard::sac::thermal::TCHPManager& TCHPManagerWrite = writeScenario.TCHPmanager();
  TCHPManagerWrite.setMinTCHP(1);
  TCHPManagerWrite.setMaxTCHP(4);
  
  casaWizard::sac::thermal::OptimizedTCHP optim(10, 4);
  TCHPManagerWrite.addOptimizedTCHP(optim);

  casaWizard::ScenarioWriter writer{"scenario.dat"};
  writeScenario.writeToFile(writer);

  writer.close();

  casaWizard::sac::thermal::ThermalScenario readScenario{new casaWizard::StubProjectReader()};
  casaWizard::ScenarioReader reader{"scenario.dat"};
  readScenario.readFromFile(reader);

  const casaWizard::sac::thermal::TCHPManager& TCHPManagerRead = readScenario.TCHPmanager();

  const QVector<casaWizard::sac::thermal::OptimizedTCHP>& optimized = TCHPManagerRead.optimizedTCHPs();
  ASSERT_EQ(optimized.size(), 1);
  EXPECT_EQ(optimized[0].wellId(), 10);
  EXPECT_DOUBLE_EQ(optimized[0].optimizedHeatProduction(), 4);
  EXPECT_DOUBLE_EQ(TCHPManagerRead.minTCHP(), 1);
  EXPECT_DOUBLE_EQ(TCHPManagerRead.maxTCHP(), 4);

  casaWizard::ScenarioWriter writerLoaded{"scenarioAgain.dat"};
  readScenario.writeToFile(writerLoaded);
  writerLoaded.close();
  expectFileEq("scenario.dat", "scenarioAgain.dat");
}

TEST(ThermalScenarioTest, testGetTCHPOfClosestWell)
{
   // Given
   casaWizard::sac::thermal::ThermalScenario scenario{new casaWizard::StubProjectReader()};
   scenario.calibrationTargetManager().addWell("ActiveWell1", 1000, 1000);
   scenario.calibrationTargetManager().addWell("ActiveWell2", 3000, 3000);
   scenario.calibrationTargetManager().addWell("Non-ActiveWell", 2000, 2000);
   scenario.calibrationTargetManager().setWellIsActive(false, 2);

   scenario.TCHPmanager().addOptimizedTCHP(casaWizard::sac::thermal::OptimizedTCHP(0, 1.1));
   scenario.TCHPmanager().addOptimizedTCHP(casaWizard::sac::thermal::OptimizedTCHP(1, 2.2));
   scenario.TCHPmanager().addOptimizedTCHP(casaWizard::sac::thermal::OptimizedTCHP(2, 3.3));

   // When
   int closestWellID = -1;
   const double TCHPOfClosestWell = scenario.getTCHPOfClosestWell(2.2, 2.2, closestWellID);

   // Then
   EXPECT_EQ(closestWellID, 1);
   EXPECT_DOUBLE_EQ(TCHPOfClosestWell, 2.2);
}


