#include "model/wellTrajectoryManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

namespace casasac = casaWizard::sac;

TEST(WellTrajectoryManagerTest, testAddTrajectory)
{
  casasac::WellTrajectoryManager mgr{};

  const QString property{"Temperature"};
  const int wellIndex{15};
  mgr.addWellTrajectory(wellIndex, property);

  const QVector<casasac::WellTrajectory> baseRunWells = mgr.trajectoriesType(casasac::TrajectoryType::Original1D);
  const QVector<casasac::WellTrajectory> bestMatchWells = mgr.trajectoriesType(casasac::TrajectoryType::Optimized1D);
  const QVector<casasac::WellTrajectory> base3dWells = mgr.trajectoriesType(casasac::TrajectoryType::Original3D);
  const QVector<casasac::WellTrajectory> optimized3dWells = mgr.trajectoriesType(casasac::TrajectoryType::Optimized3D);

  ASSERT_EQ( 1, baseRunWells.size());
  EXPECT_EQ( baseRunWells[0].property(), property);
  EXPECT_EQ( baseRunWells[0].wellIndex(), wellIndex);

  ASSERT_EQ( 1, bestMatchWells.size());
  EXPECT_EQ( bestMatchWells[0].property(), property);
  EXPECT_EQ( bestMatchWells[0].wellIndex(), wellIndex);

  ASSERT_EQ( 1, base3dWells.size());
  EXPECT_EQ( base3dWells[0].property(), property);
  EXPECT_EQ( base3dWells[0].wellIndex(), wellIndex);

  ASSERT_EQ( 1, optimized3dWells.size());
  EXPECT_EQ( optimized3dWells[0].property(), property);
  EXPECT_EQ( optimized3dWells[0].wellIndex(), wellIndex);
}

TEST(WellTrajectoryManagerTest, testSelectFromWell)
{
  casasac::WellTrajectoryManager mgr{};

  mgr.addWellTrajectory(0, "Temperature");
  mgr.addWellTrajectory(1, "Temperature");
  mgr.addWellTrajectory(0, "VRe");
  mgr.addWellTrajectory(2, "VRe");

  const QVector<casasac::WellTrajectory> baseRunInWell = mgr.trajectoriesInWell({0}, {"Temperature","VRe"})[casasac::TrajectoryType::Original1D];

  ASSERT_EQ( baseRunInWell.size(), 2);

  EXPECT_EQ( baseRunInWell[0].wellIndex(), 0);
  EXPECT_EQ( baseRunInWell[1].wellIndex(), 0);
  EXPECT_EQ( baseRunInWell[0].property(), "Temperature");
  EXPECT_EQ( baseRunInWell[1].property(), "VRe");
}

TEST(WellTrajectoryManagerTest, testSelectMultiWell)
{
  casasac::WellTrajectoryManager mgr{};

  mgr.addWellTrajectory(0, "Temperature");
  mgr.addWellTrajectory(1, "Temperature");
  mgr.addWellTrajectory(1, "VRe");
  mgr.addWellTrajectory(2, "SonicSlowness");

  const QVector<casasac::WellTrajectory> baseRunInWell = mgr.trajectoriesInWell({0,2}, {"Temperature","VRe", "SonicSlowness"})[casasac::TrajectoryType::Original1D];

  ASSERT_EQ( 2, baseRunInWell.size());

  EXPECT_EQ(0, baseRunInWell[0].wellIndex());
  EXPECT_EQ(2, baseRunInWell[1].wellIndex());
  EXPECT_EQ("Temperature", baseRunInWell[0].property());
  EXPECT_EQ("SonicSlowness", baseRunInWell[1].property());
}


TEST(WellTrajectoryManagerTest, testWriteToFile)
{
  casasac::WellTrajectoryManager mgr{};

  const QString property{"Temperature"};
  const int wellIndex1{15};
  mgr.addWellTrajectory(wellIndex1, property);

  const int wellIndex2{16};
  mgr.addWellTrajectory(wellIndex2, property);

  mgr.setTrajectoryData(casasac::Original1D,      0, {1, 2, 3}, {4, 5, 6});
  mgr.setTrajectoryData(casasac::Optimized1D, 0, {1, 2, 3}, {7, 8, 9});
  mgr.setTrajectoryData(casasac::Original3D,      0, {1, 2, 3}, {10, 11, 12});
  mgr.setTrajectoryData(casasac::Optimized3D, 0, {1, 2, 3}, {13, 14, 15});

  casaWizard::ScenarioWriter writer{"wellTrajectoryManagerActual.dat"};

  mgr.writeToFile(writer);
  writer.close();

  expectFileEq("wellTrajectoryManagerActual.dat", "wellTrajectoryManager.dat");
}

TEST(WellTrajectoryManagerTest, testReadFromFile)
{
  casasac::WellTrajectoryManager mgr{};
  casaWizard::ScenarioReader reader{"wellTrajectoryManager.dat"};

  mgr.readFromFile(reader);

  const QVector<QVector<casasac::WellTrajectory>> trajectoriesInWell = mgr.trajectoriesInWell({15}, {"Temperature"});

  const casasac::WellTrajectory baseRunWell = trajectoriesInWell[casasac::TrajectoryType::Original1D][0];
  const casasac::WellTrajectory bestMatchWell = trajectoriesInWell[casasac::TrajectoryType::Optimized1D][0];
  const casasac::WellTrajectory base3dWell = trajectoriesInWell[casasac::TrajectoryType::Original3D][0];
  const casasac::WellTrajectory optimized3dWell = trajectoriesInWell[casasac::TrajectoryType::Optimized3D][0];

  EXPECT_EQ(baseRunWell.property().toStdString(), "Temperature");
  EXPECT_EQ(baseRunWell.wellIndex(), 15);
  EXPECT_EQ(baseRunWell.trajectoryIndex(), 0);
  QVector<double> depth = {1, 2, 3};
  QVector<double> baseRun = {4, 5, 6};
  QVector<double> bestMatch =  {7, 8, 9};
  QVector<double> base3d = {10, 11, 12};
  QVector<double> optimized3d = {13, 14, 15};
  for (int i=0; i<3; ++i)
  {
    EXPECT_EQ(baseRunWell.depth()[i], depth[i]);
    EXPECT_EQ(baseRunWell.value()[i], baseRun[i]);
    EXPECT_EQ(bestMatchWell.depth()[i], depth[i]);
    EXPECT_EQ(bestMatchWell.value()[i], bestMatch[i]);

    EXPECT_EQ(base3dWell.depth()[i], depth[i]);
    EXPECT_EQ(base3dWell.value()[i], base3d[i]);
    EXPECT_EQ(optimized3dWell.depth()[i], depth[i]);
    EXPECT_EQ(optimized3dWell.value()[i], optimized3d[i]);
  }
}

TEST(WellTrajectoryManagerTest, testWriteReadEmptyResults)
{
  casasac::WellTrajectoryManager managerWrite{};
  managerWrite.addWellTrajectory(1, "Temperature");
  casaWizard::ScenarioWriter writer{"wellTrajectoryManagerEmpty.dat"};
  managerWrite.writeToFile(writer);
  writer.close();

  casasac::WellTrajectoryManager managerRead{};
  casaWizard::ScenarioReader reader{"wellTrajectoryManagerEmpty.dat"};
  managerRead.readFromFile(reader);

  const QVector<QVector<casasac::WellTrajectory>> trajectoriesInWell = managerRead.trajectoriesInWell({1}, {"Temperature"});
  const casasac::WellTrajectory baseRunWell   = trajectoriesInWell[casasac::Original1D][0];
  const casasac::WellTrajectory bestMatchWell = trajectoriesInWell[casasac::Optimized1D][0];

  EXPECT_EQ(baseRunWell.depth().size(), 0);
  EXPECT_EQ(baseRunWell.value().size(), 0);
  EXPECT_EQ(bestMatchWell.depth().size(), 0);
  EXPECT_EQ(bestMatchWell.value().size(), 0);
}
