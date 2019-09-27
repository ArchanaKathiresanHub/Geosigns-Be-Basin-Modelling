#include "model/predictionTargetManager.h"

#include "../stub/stubProjectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>
#include <memory>

class TestPredictionTargetManager : public ::testing::Test
{
protected:
  casaWizard::StubProjectReader projectReader_;
};

TEST_F( TestPredictionTargetManager, TestWriteToFile )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addDepthTarget("Temperature", 1, 2, 3, 11);
  manager.addDepthTarget("VRe", 4, 5, 6, 12);
  manager.addSurfaceTarget("Temperature", 7, 8, "Layer", 10);

  casaWizard::ScenarioWriter writer{"predictionTargetManagerActual.dat"};
  manager.writeToFile(writer);

  writer.close();

  expectFileEq("predictionTargetManager.dat", "predictionTargetManagerActual.dat");
}

TEST_F( TestPredictionTargetManager, TestReadFromFile )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  casaWizard::ScenarioReader reader{"predictionTargetManager.dat"};
  manager.readFromFile(reader);

  const QVector<casaWizard::ua::PredictionTargetDepth> depthTargets = manager.depthTargets();

  ASSERT_EQ(depthTargets.size(), 2);

  EXPECT_DOUBLE_EQ(depthTargets[0].x(), 1);
  EXPECT_DOUBLE_EQ(depthTargets[0].y(), 2);
  EXPECT_DOUBLE_EQ(depthTargets[0].z(), 3);
  EXPECT_DOUBLE_EQ(depthTargets[0].age(), 11);

  EXPECT_DOUBLE_EQ(depthTargets[1].x(), 4);
  EXPECT_DOUBLE_EQ(depthTargets[1].y(), 5);
  EXPECT_DOUBLE_EQ(depthTargets[1].z(), 6);
  EXPECT_DOUBLE_EQ(depthTargets[1].age(), 12);

  const QVector<casaWizard::ua::PredictionTargetSurface> surfaceTargets = manager.surfaceTargets();
  ASSERT_EQ(1, surfaceTargets.size());
  EXPECT_EQ(7, surfaceTargets[0].x());
  EXPECT_EQ(8, surfaceTargets[0].y());
  EXPECT_EQ("Layer", surfaceTargets[0].layerName());
  EXPECT_EQ(10, surfaceTargets[0].age());
}

TEST_F( TestPredictionTargetManager, TestCopyDepthTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addDepthTarget("Temperature", 1, 2, 3,4);
  manager.copyDepthTarget(0);

  const QVector<casaWizard::ua::PredictionTargetDepth> targets = manager.depthTargets();
  ASSERT_EQ(2, targets.size());
  EXPECT_EQ(1, targets[1].x());
  EXPECT_EQ(2, targets[1].y());
  EXPECT_EQ(3, targets[1].z());
  EXPECT_EQ(4, targets[1].age());
}

TEST_F( TestPredictionTargetManager, TestCopyNonExistingDepthTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addDepthTarget("Temperature", 1, 2, 3, 4);
  manager.copyDepthTarget(1);

  const QVector<casaWizard::ua::PredictionTargetDepth> targets = manager.depthTargets();
  ASSERT_EQ(1, targets.size());
}

TEST_F( TestPredictionTargetManager, TestCopySurfaceTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer", 3);
  manager.copySurfaceTarget(0);

  const QVector<casaWizard::ua::PredictionTargetSurface> targets = manager.surfaceTargets();
  ASSERT_EQ(2, targets.size());

  EXPECT_EQ(1, targets[1].x());
  EXPECT_EQ(2, targets[1].y());
  EXPECT_EQ("Layer", targets[1].layerName());
  EXPECT_EQ(3, targets[1].age());
}

TEST_F( TestPredictionTargetManager, TestCopyNonExistingSurfaceTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer", 3);
  manager.copySurfaceTarget(1);

  const QVector<casaWizard::ua::PredictionTargetSurface> targets = manager.surfaceTargets();
  ASSERT_EQ(1, targets.size());
}

TEST_F( TestPredictionTargetManager, TestRemoveDepthTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addDepthTarget("Temperature", 1, 2, 3, 4);
  manager.addDepthTarget("Temperature", 5, 6, 7, 8);
  manager.removeDepthTarget(0);

  const QVector<casaWizard::ua::PredictionTargetDepth> targets = manager.depthTargets();
  ASSERT_EQ(1, targets.size());
  EXPECT_EQ(5, targets[0].x());
  EXPECT_EQ(6, targets[0].y());
  EXPECT_EQ(7, targets[0].z());
  EXPECT_EQ(8, targets[0].age());
}

TEST_F( TestPredictionTargetManager, TestRemoveNonExistingDepthTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addDepthTarget("Temperature", 1, 2, 3, 4);
  manager.addDepthTarget("Temperature", 5, 6, 7, 8);
  manager.removeDepthTarget(3);

  const QVector<casaWizard::ua::PredictionTargetDepth> targets = manager.depthTargets();
  ASSERT_EQ(2, targets.size());
}

TEST_F( TestPredictionTargetManager, TestRemoveSurfaceTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer", 3);
  manager.addSurfaceTarget("Temperature", 4, 5, "Layer", 6);
  manager.removeSurfaceTarget(0);

  const QVector<casaWizard::ua::PredictionTargetSurface> targets = manager.surfaceTargets();
  ASSERT_EQ(1, targets.size());
  EXPECT_EQ(4, targets[0].x());
  EXPECT_EQ(5, targets[0].y());
  EXPECT_EQ("Layer", targets[0].layerName());
  EXPECT_EQ(6, targets[0].age());
}

TEST_F( TestPredictionTargetManager, TestRemoveSurfaceTargetNonExisting )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer", 3);
  manager.addSurfaceTarget("Temperature", 4, 5, "Layer", 6);
  manager.removeSurfaceTarget(3);

  const QVector<casaWizard::ua::PredictionTargetSurface> targets = manager.surfaceTargets();
  ASSERT_EQ(2, targets.size());
}

TEST_F( TestPredictionTargetManager, TestTimeSeriesFunctionalitiesDepthTargets )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addDepthTarget("Temperature", 1, 2, 3); // With time series
  manager.setDepthTargetHasTimeSeries(0, true);
  manager.addDepthTarget("VRe", 4, 5, 6); // With time series
  manager.setDepthTargetHasTimeSeries(1, true);
  manager.addDepthTarget("VRe", 4, 5, 6); // Without time series

  const int sizeOfTimeSeries = projectReader_.agesFromMajorSnapshots().size();
  const QVector<int> sizesExpected = {sizeOfTimeSeries, sizeOfTimeSeries, 1};

  int i = 0;
  for (const int& sExpected : sizesExpected)
  {
    const int sActual = manager.sizeOfPredictionTargetWithTimeSeries(i);
    EXPECT_EQ(sExpected, sActual);
    ++i;
  }

  const QVector<int> indexCumulativeSizeAllExpected = {0, sizeOfTimeSeries, 2*sizeOfTimeSeries};

  for (int i = 0; i < 3; ++i)
  {
    const int csExpected = indexCumulativeSizeAllExpected[i];
    const int csActual = manager.indexCumulativePredictionTarget(i);
    EXPECT_EQ(csExpected, csActual);
  }

  EXPECT_EQ(17, manager.amountIncludingTimeSeries());
}

TEST_F( TestPredictionTargetManager, TestTimeSeriesFunctionalitiesSurfaceTargets )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer"); // With time series
  manager.setSurfaceTargetHasTimeSeries(0, true);
  manager.addSurfaceTarget("VRe", 4, 5, "Layer"); // With time series
  manager.setSurfaceTargetHasTimeSeries(1, true);
  manager.addSurfaceTarget("VRe", 4, 5, "Layer"); // Without time series

  const int sizeOfTimeSeries = projectReader_.agesFromMajorSnapshots().size();
  const QVector<int> sizesExpected = {sizeOfTimeSeries, sizeOfTimeSeries, 1};

  int i = 0;
  for (const int& sExpected : sizesExpected)
  {
    const int sActual = manager.sizeOfPredictionTargetWithTimeSeries(i);
    EXPECT_EQ(sExpected, sActual);
    ++i;
  }

  const QVector<int> indexCumulativeSizeAllExpected = {0, sizeOfTimeSeries, 2*sizeOfTimeSeries};

  for (int i = 0; i < 3; ++i)
  {
    const int csExpected = indexCumulativeSizeAllExpected[i];
    const int csActual = manager.indexCumulativePredictionTarget(i);
    EXPECT_EQ(csExpected, csActual);
  }

  EXPECT_EQ(17, manager.amountIncludingTimeSeries());
}

TEST_F( TestPredictionTargetManager, TestTimeSeriesFunctionalitiesMixedTargets )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer"); // With time series
  manager.setSurfaceTargetHasTimeSeries(0, true);
  manager.addDepthTarget("VRe", 4, 5, 6); // With time series
  manager.setDepthTargetHasTimeSeries(0, true);

  const int sizeOfTimeSeries = projectReader_.agesFromMajorSnapshots().size();
  const QVector<int> sizesExpected = {sizeOfTimeSeries, sizeOfTimeSeries};

  int i = 0;
  for (const int& sExpected : sizesExpected)
  {
    const int sActual = manager.sizeOfPredictionTargetWithTimeSeries(i);
    EXPECT_EQ(sExpected, sActual);
    ++i;
  }

  const QVector<int> indexCumulativeSizeAllExpected = {0, sizeOfTimeSeries};

  for (int i = 0; i < 2; ++i)
  {
    const int csExpected = indexCumulativeSizeAllExpected[i];
    const int csActual = manager.indexCumulativePredictionTarget(i);
    EXPECT_EQ(csExpected, csActual);
  }

  EXPECT_EQ(16, manager.amountIncludingTimeSeries());
}

TEST_F( TestPredictionTargetManager, TestAddSurfacePredictionTarget )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer");

  const QVector<casaWizard::ua::PredictionTargetSurface> target = manager.surfaceTargets();

  ASSERT_EQ(1, target.size());
  EXPECT_EQ(1, target[0].x());
  EXPECT_EQ(2, target[0].y());
  EXPECT_EQ("Layer", target[0].layerName());
}

TEST_F( TestPredictionTargetManager, TestAddSurfaceIncorrectLayerName )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addSurfaceTarget("Temperature", 1, 2, "Non-existing name");

  const QVector<casaWizard::ua::PredictionTargetSurface> targets = manager.surfaceTargets();

  ASSERT_EQ(1, targets.size());
  EXPECT_EQ("Layer", targets[0].layerName());
}

TEST_F( TestPredictionTargetManager, TestPredictionTargets )
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);

  manager.addSurfaceTarget("Temperature", 1, 2, "Layer");
  manager.addDepthTarget("Temperature", 2, 2, 3, 11);
  manager.addDepthTarget("VRe", 4, 5, 6, 12);

  QVector<const casaWizard::ua::PredictionTarget*> allTargets = manager.predictionTargets();
  ASSERT_EQ(3, allTargets.size());
  ASSERT_TRUE(dynamic_cast<const casaWizard::ua::PredictionTargetDepth*>(allTargets[0]));
  ASSERT_TRUE(dynamic_cast<const casaWizard::ua::PredictionTargetDepth*>(allTargets[1]));
  ASSERT_TRUE(dynamic_cast<const casaWizard::ua::PredictionTargetSurface*>(allTargets[2]));

  EXPECT_EQ(2, allTargets[0]->x());
  EXPECT_EQ(4, allTargets[1]->x());
  EXPECT_EQ(1, allTargets[2]->x());
}

TEST_F( TestPredictionTargetManager, TestPredictionTargetsWithTimeSeries)
{
  casaWizard::ua::PredictionTargetManager manager(projectReader_);
  manager.addDepthTarget("Temperature", 2, 2, 3, 11);
  manager.addDepthTarget("VRe", 4, 5, 6, 12);
  manager.setDepthTargetHasTimeSeries(0, true);
  manager.addSurfaceTarget("Temperature", 1, 2, "Layer");
  manager.setSurfaceTargetHasTimeSeries(0, true);

  QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargetsIncludingTimeSeries();

  ASSERT_EQ(17, targets.size());

  EXPECT_EQ(2, targets[0]->x());
  EXPECT_EQ(2, targets[7]->x());
  EXPECT_EQ(4, targets[8]->x());
  EXPECT_EQ(1, targets[9]->x());
  EXPECT_EQ(1, targets[16]->x());
}
