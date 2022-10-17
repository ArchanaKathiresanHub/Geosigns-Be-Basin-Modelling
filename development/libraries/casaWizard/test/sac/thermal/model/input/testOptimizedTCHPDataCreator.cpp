#include "model/input/OptimizedTCHPDataCreator.h"

#include "model/ThermalScenario.h"
#include "model/OptimizedTCHP.h"
#include "stubProjectReader.h"

#include <gtest/gtest.h>

TEST( OptimizedTCHPDataCreator, testReadTCHPFile )
{
  casaWizard::sac::thermal::ThermalScenario scenario{new casaWizard::StubProjectReader()};
  casaWizard::sac::thermal::TCHPManager& TCHPManager{scenario.TCHPmanager()};
  casaWizard::CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
  const int wellIndex1 = ctManager.addWell("Well1", 0, 0);
  const int wellIndex2 = ctManager.addWell("Well2", 1, 1);

  const QString iterationPath{"TCHPData"};
  casaWizard::sac::thermal::OptimizedTCHPDataCreator dataCreator{scenario, iterationPath};

  dataCreator.read();

  const QVector<casaWizard::sac::thermal::OptimizedTCHP> optimizedTCHPs = TCHPManager.optimizedTCHPs();

  ASSERT_EQ( optimizedTCHPs.size(), ctManager.wells().size());

  QVector<int> wellIndexExpected = {wellIndex1, wellIndex2};
  QVector<double> heatProdExpected = {60, 6};

  int index = 0;
  for (const casaWizard::sac::thermal::OptimizedTCHP& optimizedActual : optimizedTCHPs)
  {
    EXPECT_EQ( wellIndexExpected[index], optimizedActual.wellId()) << "Mismatch at index " + index;
    EXPECT_DOUBLE_EQ( heatProdExpected[index], optimizedActual.optimizedHeatProduction()) << "Mismatch at index " + index;
    ++index;
  }
}
