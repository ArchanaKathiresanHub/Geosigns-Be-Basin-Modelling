#include "model/case3DTrajectoryConvertor.h"

#include "model/input/case3DTrajectoryReader.h"
#include "model/sacScenario.h"

#include "stubProjectReader.h"


#include <gtest/gtest.h>

TEST(Case3DTrajectoryConvertorTest, testConvert)
{
  casaWizard::sac::Case3DTrajectoryReader reader("input/wellDataCase3D.csv");
  reader.read();

  casaWizard::sac::SACScenario scenario(std::unique_ptr<casaWizard::ProjectReader>(new casaWizard::StubProjectReader()));


  casaWizard::CalibrationTargetManager& calibrationTargetManager = scenario.calibrationTargetManager();

  calibrationTargetManager.addWell("Well0", 186900, 610870);
  calibrationTargetManager.addWell("Well1", 191772, 615200);
  calibrationTargetManager.addWell("Well2", 190619, 615337);
  calibrationTargetManager.addWell("Well3", 191218, 612824);
  calibrationTargetManager.addWell("Well4", 182933, 607825);
  calibrationTargetManager.addWell("Well5", 181866, 615170);
  calibrationTargetManager.addWell("Well6", 199032, 614340);
  calibrationTargetManager.addWell("Well7", 180517, 612131);
  calibrationTargetManager.addWell("Well8", 201700, 610300);
  calibrationTargetManager.addWell("Well9", 199250, 604100);
  calibrationTargetManager.addWell("Well10", 191500, 604400);
  const QStringList properties = {"TwoWayTime","BulkDensity","SonicSlowness"};

  casaWizard::sac::WellTrajectoryManager& wellTrajectoryManager = scenario.wellTrajectoryManager();
  for (const QString prop : properties)
  {
    for (const casaWizard::Well& well : calibrationTargetManager.wells())
    {
      wellTrajectoryManager.addWellTrajectory(well.id(), prop);
    }
  }

  casaWizard::sac::case3DTrajectoryConvertor::convertToScenario(reader, scenario);

  QVector<casaWizard::sac::WellTrajectory> trajectories1 = wellTrajectoryManager.trajectoriesInWell({0}, {"BulkDensity"})[casaWizard::sac::Optimized3D];
  ASSERT_EQ(1, trajectories1.size());
  ASSERT_EQ(47, trajectories1[0].value().size());
  EXPECT_DOUBLE_EQ(1679.01, trajectories1[0].value()[0]);
  EXPECT_DOUBLE_EQ(2093.82, trajectories1[0].value()[46]);

  QVector<casaWizard::sac::WellTrajectory> trajectories2 = wellTrajectoryManager.trajectoriesInWell({8}, {"SonicSlowness"})[casaWizard::sac::Optimized3D];
  ASSERT_EQ(1, trajectories2.size());
  ASSERT_EQ(47, trajectories2[0].value().size());
  EXPECT_DOUBLE_EQ(669.841, trajectories2[0].value()[0]);
  EXPECT_DOUBLE_EQ(271.885, trajectories2[0].value()[46]);
  EXPECT_DOUBLE_EQ(4546.99, trajectories2[0].depth()[45]);

  QVector<casaWizard::sac::WellTrajectory> trajectories3 = wellTrajectoryManager.trajectoriesInWell({10}, {"TwoWayTime"})[casaWizard::sac::Optimized3D];
  ASSERT_EQ(1, trajectories3.size());
  ASSERT_EQ(47, trajectories3[0].value().size());
  EXPECT_DOUBLE_EQ(4.99401, trajectories3[0].depth()[0]);
  EXPECT_DOUBLE_EQ(4515.7, trajectories3[0].depth()[46]);
  EXPECT_DOUBLE_EQ(2863.31, trajectories3[0].value()[45]);
}
