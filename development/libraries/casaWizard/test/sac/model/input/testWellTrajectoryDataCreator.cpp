#include "model/input/wellTrajectoryDataCreator.h"

#include "model/sacScenario.h"
#include "model/wellTrajectory.h"
#include "stubProjectReader.h"

#include <gtest/gtest.h>

TEST( WellTrajectoryDataCreatorTest, testReadTrack1DFile )
{
  casaWizard::sac::SACScenario scenario{std::unique_ptr<casaWizard::ProjectReader>(new casaWizard::StubProjectReader())};
  scenario.setProject3dFilePath("Project.project3d");

  casaWizard::CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
  const int wellIndex = ctManager.addWell("Well1", 0, 0);
  casaWizard::sac::WellTrajectoryManager& wellTrajectoryManager = scenario.wellTrajectoryManager();
  wellTrajectoryManager.addWellTrajectory(wellIndex, "Temperature");

  const QString iterationPath{"TrajectoryData"};
  casaWizard::sac::WellTrajectoryDataCreator dataCreator{scenario, iterationPath};

  dataCreator.read();

  const QVector<casaWizard::sac::WellTrajectory> trajectories = wellTrajectoryManager.trajectoriesType(casaWizard::sac::TrajectoryType::Base1D);

  QVector<double> depthActual = trajectories[0].depth();
  QVector<double> valueActual = trajectories[0].value();

  QVector<double> depthExpected = {0, 100};
  QVector<double> valueExpected = {10, 20};

  ASSERT_EQ(depthActual.size(), depthExpected.size());
  ASSERT_EQ(valueActual.size(), valueExpected.size());
  ASSERT_EQ(depthActual.size(), valueActual.size());

  const int n = depthActual.size();

  for( int i = 0; i< n; ++i )
  {
    EXPECT_EQ(depthActual[i], depthExpected[i]) << "Mismatch for depth at index " << i;
    EXPECT_EQ(valueActual[i], valueExpected[i]) << "Mismatch for value at index " << i;
  }
}
