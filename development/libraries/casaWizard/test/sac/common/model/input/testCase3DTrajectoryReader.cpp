#include "model/input/case3DTrajectoryReader.h"

#include <gtest/gtest.h>

TEST(Case3DTrajectoryReaderTest, testProperties)
{
  QString filename = "wellDataCase3D.csv";
  casaWizard::sac::Case3DTrajectoryReader reader(filename);
  reader.read();

  const QStringList propertiesActual = reader.properties();
  const QStringList propertiesExpected = {"TwoWayTime","BulkDensity","SonicSlowness"};

  ASSERT_EQ(propertiesExpected.size(), propertiesActual.size());

  for (int i=0; i<propertiesActual.size(); ++i)
  {
    EXPECT_EQ(propertiesExpected[i].toStdString(), propertiesActual[i].toStdString());
  }
}

TEST(Case3DTrajectoryReaderTest, testReadXY)
{
  QString filename = "wellDataCase3D.csv";
  casaWizard::sac::Case3DTrajectoryReader reader(filename);
  reader.read();

  const QVector<double> x = reader.x();
  const QVector<double> y = reader.y();

  ASSERT_EQ(517, x.size());
  ASSERT_EQ(517, y.size());

  EXPECT_DOUBLE_EQ(186900, x[0]);
  EXPECT_DOUBLE_EQ(610870, y[0]);

  EXPECT_DOUBLE_EQ(191500, *(x.end()-1));
  EXPECT_DOUBLE_EQ(604400, *(y.end()-1));
}

TEST(Case3DTrajectoryReaderTest, testReadDepth)
{
  QString filename = "wellDataCase3D.csv";
  casaWizard::sac::Case3DTrajectoryReader reader(filename);
  reader.read();

  const QVector<double> depth = reader.depth();

  ASSERT_EQ(517, depth.size());

  EXPECT_DOUBLE_EQ(4.74895, depth[0]);

  EXPECT_DOUBLE_EQ(4515.7, *(depth.end()-1));
}


TEST(Case3DTrajectoryReaderTest, testReadProperty)
{
  QString filename = "wellDataCase3D.csv";
  casaWizard::sac::Case3DTrajectoryReader reader(filename);
  reader.read();

  const QVector<double> twoWayTime    = reader.readProperty("TwoWayTime");
  const QVector<double> bulkDensity   = reader.readProperty("BulkDensity");
  const QVector<double> sonicSlowness = reader.readProperty("SonicSlowness");

  ASSERT_EQ(517, twoWayTime.size());
  ASSERT_EQ(517, bulkDensity.size());
  ASSERT_EQ(517, sonicSlowness.size());

  EXPECT_DOUBLE_EQ(6.38774, twoWayTime[0]);
  EXPECT_DOUBLE_EQ(2154.46, bulkDensity[38]);
  EXPECT_DOUBLE_EQ(342.725, sonicSlowness[153]);
}
