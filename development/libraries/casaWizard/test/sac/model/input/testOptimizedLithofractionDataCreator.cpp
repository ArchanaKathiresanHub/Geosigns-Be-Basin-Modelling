#include "model/input/optimizedLithofractionDataCreator.h"

#include "model/sacScenario.h"
#include "model/optimizedLithofraction.h"
#include "stubProjectReader.h"

#include <gtest/gtest.h>

TEST( OptimizedLithofractionDataCreator, testReadLithofractionFile )
{
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  casaWizard::sac::LithofractionManager& lithofractionManager{scenario.lithofractionManager()};
  casaWizard::CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
  const int wellIndex1 = ctManager.addWell("Well1", 0, 0);
  lithofractionManager.addLithofraction("Layer1");
  lithofractionManager.addLithofraction("Layer2");

  const QString iterationPath{"LithofractionData"};
  casaWizard::sac::OptimizedLithofractionDataCreator dataCreator{scenario, iterationPath};

  dataCreator.read();

  const QVector<casaWizard::sac::OptimizedLithofraction> optimizedLithofractions = lithofractionManager.optimizedLithofractions();

  ASSERT_EQ( optimizedLithofractions.size(), ctManager.wells().size() * lithofractionManager.lithofractions().size());

  QVector<int> wellIndexExpected = {wellIndex1, wellIndex1};
  QVector<int> lithofractionIdExpected = {0, 1};
  QVector<double> firstComponentExpected = {10.10, 20.20};
  QVector<double> secondComponentExpected = {0.45, 0.83};

  int index = 0;
  for (const casaWizard::sac::OptimizedLithofraction& optimizedActual : optimizedLithofractions)
  {
    EXPECT_EQ( wellIndexExpected[index], optimizedActual.wellId()) << "Mismatch at index " + index;
    EXPECT_EQ( lithofractionIdExpected[index], optimizedActual.lithofractionId()) << "Mismatch at index " + index;
    EXPECT_DOUBLE_EQ( firstComponentExpected[index], optimizedActual.optimizedPercentageFirstComponent()) << "Mismatch at index " + index;
    EXPECT_DOUBLE_EQ( secondComponentExpected[index], optimizedActual.optimizedFractionSecondComponent()) << "Mismatch at index " + index;
    ++index;
  }
}
