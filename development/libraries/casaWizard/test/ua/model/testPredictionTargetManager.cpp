#include "model/predictionTargetManager.h"

#include "model/input/cmbMapReader.h"
#include "model/SurfaceToDepthConverter.h"
#include "../stub/stubProjectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>
#include <memory>
#include <utility>

class TestPredictionTargetManager : public ::testing::Test
{
public:
   void addDepthTargetToManager(casaWizard::ua::PredictionTargetManager& manager)
   {
      manager.addDepthTarget(1, 2, 3, {"Temperature"}, 4);
   }

   void addMultipleDepthTargetsToManager(casaWizard::ua::PredictionTargetManager& manager)
   {
      manager.addDepthTarget(1, 2, 3, {"Temperature"}, 4);
      manager.addDepthTarget(5, 6, 7, {"Temperature", "VRe"}, 8);
   }

   void addSurfaceTargetToManager(casaWizard::ua::PredictionTargetManager& manager)
   {
      manager.addSurfaceTarget(2, 2, "Water_Bottom", {"Temperature"}, 3);
   }

   void addMultipleSurfaceTargetsToManager(casaWizard::ua::PredictionTargetManager& manager)
   {
      manager.addSurfaceTarget(1, 2, "Water_Bottom", {"Temperature"}, 3);
      manager.addSurfaceTarget(4, 5, "Water_Bottom", {"Temperature"}, 6);
   }

   void SetUp() override
   {
      SurfaceToDepthConverter_ = std::unique_ptr<casaWizard::SurfaceToDepthConverter>(new casaWizard::SurfaceToDepthConverter(projectReader_,casaWizard::CMBMapReader()));
      targetManager_ = std::unique_ptr<casaWizard::ua::PredictionTargetManager>(new casaWizard::ua::PredictionTargetManager(projectReader_,*SurfaceToDepthConverter_));
   }

   casaWizard::ua::PredictionTargetManager& targetManager()
   {
      return *targetManager_;
   }

protected:
   casaWizard::StubProjectReader projectReader_;
   std::unique_ptr<casaWizard::ua::PredictionTargetManager> targetManager_;
   std::unique_ptr<casaWizard::SurfaceToDepthConverter> SurfaceToDepthConverter_;

};

TEST_F( TestPredictionTargetManager, TestWriteToFile )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   manager.addDepthTarget(1, 2, 3, {"Temperature", "VRe"}, 11);
   manager.addDepthTarget(4, 5, 6, {"VRe"}, 12);
   manager.addSurfaceTarget(7, 8, "Water_Bottom", {"Temperature"}, 10);

   casaWizard::ScenarioWriter writer{"predictionTargetManagerActual.dat"};
   manager.writeToFile(writer);

   writer.close();

   expectFileEq("predictionTargetManagerv2.dat", "predictionTargetManagerActual.dat");
}

TEST_F( TestPredictionTargetManager, Readv2 )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   casaWizard::ScenarioReader reader{"predictionTargetManagerv2.dat"};
   manager.readFromFile(reader);

   const QVector<const casaWizard::ua::PredictionTarget*> predictionTargets = manager.predictionTargets();

   ASSERT_EQ(predictionTargets.size(), 3);

   EXPECT_DOUBLE_EQ(predictionTargets[0]->x(), 1);
   EXPECT_DOUBLE_EQ(predictionTargets[0]->y(), 2);
   EXPECT_DOUBLE_EQ(predictionTargets[0]->z(), 3);
   EXPECT_DOUBLE_EQ(predictionTargets[0]->age(), 11);

   EXPECT_DOUBLE_EQ(predictionTargets[1]->x(), 4);
   EXPECT_DOUBLE_EQ(predictionTargets[1]->y(), 5);
   EXPECT_DOUBLE_EQ(predictionTargets[1]->z(), 6);
   EXPECT_DOUBLE_EQ(predictionTargets[1]->age(), 12);

   EXPECT_EQ(7, predictionTargets[2]->x());
   EXPECT_EQ(8, predictionTargets[2]->y());
   EXPECT_EQ("Water_Bottom", predictionTargets[2]->surfaceName());
   EXPECT_EQ("\"Layer1\"", predictionTargets[2]->variable().toStdString());
   EXPECT_EQ(10, predictionTargets[2]->age());
}

TEST_F( TestPredictionTargetManager, TestReadFromFile )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   casaWizard::ScenarioReader reader{"predictionTargetManager.dat"};
   manager.readFromFile(reader);

   const QVector<const casaWizard::ua::PredictionTarget*> predictionTargets = manager.predictionTargets();

   ASSERT_EQ(predictionTargets.size(), 3);

   EXPECT_DOUBLE_EQ(predictionTargets[0]->x(), 1);
   EXPECT_DOUBLE_EQ(predictionTargets[0]->y(), 2);
   EXPECT_DOUBLE_EQ(predictionTargets[0]->z(), 3);
   EXPECT_DOUBLE_EQ(predictionTargets[0]->age(), 11);

   EXPECT_DOUBLE_EQ(predictionTargets[1]->x(), 4);
   EXPECT_DOUBLE_EQ(predictionTargets[1]->y(), 5);
   EXPECT_DOUBLE_EQ(predictionTargets[1]->z(), 6);
   EXPECT_DOUBLE_EQ(predictionTargets[1]->age(), 12);

   EXPECT_EQ(7, predictionTargets[2]->x());
   EXPECT_EQ(8, predictionTargets[2]->y());
   EXPECT_EQ("Water_Bottom", predictionTargets[2]->surfaceName());
   EXPECT_EQ(10, predictionTargets[2]->age());
}

TEST_F( TestPredictionTargetManager, TestCopyDepthTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addDepthTargetToManager(manager);

   manager.copyTargets({0});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(2, targets.size());
   EXPECT_EQ(1, targets[1]->x());
   EXPECT_EQ(2, targets[1]->y());
   EXPECT_EQ(3, targets[1]->z());
   EXPECT_EQ(4, targets[1]->age());
}

TEST_F( TestPredictionTargetManager, TestCopyNonExistingDepthTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addDepthTargetToManager(manager);
   manager.copyTargets({1});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(1, targets.size());
}

TEST_F( TestPredictionTargetManager, TestCopySurfaceTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addSurfaceTargetToManager(manager);
   manager.copyTargets({0});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(2, targets.size());

   EXPECT_EQ(2, targets[1]->x());
   EXPECT_EQ(2, targets[1]->y());
   EXPECT_EQ("Water_Bottom", targets[1]->surfaceName());
   EXPECT_EQ(3, targets[1]->age());
}

TEST_F( TestPredictionTargetManager, TestCopyMultipleSurfaceTargets )
{
   casaWizard::CMBMapReader mapReader;
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleSurfaceTargetsToManager(manager);
   manager.copyTargets({0, 1});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(4, targets.size());
}


TEST_F( TestPredictionTargetManager, TestCopyNonExistingSurfaceTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addSurfaceTargetToManager(manager);
   manager.copyTargets({1});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(1, targets.size());
}

TEST_F( TestPredictionTargetManager, TestRemoveDepthTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleDepthTargetsToManager(manager);
   manager.removeTargets({0});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(1, targets.size());
   EXPECT_EQ(5, targets[0]->x());
   EXPECT_EQ(6, targets[0]->y());
   EXPECT_EQ(7, targets[0]->z());
   EXPECT_EQ(8, targets[0]->age());
}

TEST_F( TestPredictionTargetManager, TestRemoveNonExistingDepthTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleDepthTargetsToManager(manager);
   manager.removeTargets({3});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(2, targets.size());
}

TEST_F( TestPredictionTargetManager, TestRemoveSurfaceTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleSurfaceTargetsToManager(manager);
   manager.removeTargets({0});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(1, targets.size());
   EXPECT_EQ(4, targets[0]->x());
   EXPECT_EQ(5, targets[0]->y());
   EXPECT_EQ("Water_Bottom", targets[0]->surfaceName());
   EXPECT_EQ(6, targets[0]->age());
   EXPECT_EQ("Loc_2", targets[0]->locationName());
}

TEST_F( TestPredictionTargetManager, TestRemoveMultipleSurfaceTargets )
{
   casaWizard::CMBMapReader mapReader;
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleSurfaceTargetsToManager(manager);
   manager.removeTargets({0, 1});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(0, targets.size());
}

TEST_F( TestPredictionTargetManager, TestRemoveSurfaceTargetNonExisting )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleSurfaceTargetsToManager(manager);
   manager.removeTargets({3});

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();
   ASSERT_EQ(2, targets.size());
}

TEST_F( TestPredictionTargetManager, TestTimeSeriesFunctionalitiesDepthTargets )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleDepthTargetsToManager(manager); // with time series
   manager.setTargetHasTimeSeries(0, true);
   manager.setTargetHasTimeSeries(1, true);
   addDepthTargetToManager(manager); // Without time series

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

   EXPECT_EQ(25, manager.amountOfPredictionTargetWithTimeSeriesAndProperties());
}

TEST_F( TestPredictionTargetManager, TestTimeSeriesFunctionalitiesSurfaceTargets )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleSurfaceTargetsToManager(manager); // with time series
   manager.setTargetHasTimeSeries(0, true);
   manager.setTargetHasTimeSeries(1, true);
   addSurfaceTargetToManager(manager); // Without time series

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

   EXPECT_EQ(17, manager.amountOfPredictionTargetWithTimeSeriesAndProperties());
}

TEST_F( TestPredictionTargetManager, TestTimeSeriesFunctionalitiesMixedTargets )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addSurfaceTargetToManager(manager);
   manager.setTargetHasTimeSeries(0, true);
   addDepthTargetToManager(manager);
   manager.setTargetHasTimeSeries(1, true);

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

   EXPECT_EQ(16, manager.amountOfPredictionTargetWithTimeSeriesAndProperties());
}

TEST_F( TestPredictionTargetManager, TestAddSurfacePredictionTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addSurfaceTargetToManager(manager);

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();

   ASSERT_EQ(1, targets.size());
   EXPECT_EQ(2, targets[0]->x());
   EXPECT_EQ(2, targets[0]->y());
   EXPECT_EQ("Water_Bottom", targets[0]->surfaceName());
}

TEST_F( TestPredictionTargetManager, TestAddSurfaceIncorrectLayerName )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addSurfaceTargetToManager(manager);

   const QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();

   ASSERT_EQ(1, targets.size());
   EXPECT_EQ("Water_Bottom", targets[0]->surfaceName());
}

TEST_F( TestPredictionTargetManager, TestPredictionTargets )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();

   addSurfaceTargetToManager(manager);
   addMultipleDepthTargetsToManager(manager);

   QVector<const casaWizard::ua::PredictionTarget*> allTargets = manager.predictionTargets();
   ASSERT_EQ(3, allTargets.size());
   ASSERT_TRUE(dynamic_cast<const casaWizard::ua::PredictionTargetSurface*>(allTargets[0]));
   ASSERT_TRUE(dynamic_cast<const casaWizard::ua::PredictionTargetDepth*>(allTargets[1]));
   ASSERT_TRUE(dynamic_cast<const casaWizard::ua::PredictionTargetDepth*>(allTargets[2]));

   EXPECT_EQ(2, allTargets[0]->x());
   EXPECT_EQ(1, allTargets[1]->x());
   EXPECT_EQ(5, allTargets[2]->x());
}

TEST_F( TestPredictionTargetManager, TestSetDepth )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();

   addDepthTargetToManager(manager);
   manager.setTarget(0, 3, "3.0");

   QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();

   EXPECT_DOUBLE_EQ(3.0, targets[0]->z());
}

TEST_F( TestPredictionTargetManager, TestSwitchSurfaceToDepthTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();

   addSurfaceTargetToManager(manager);
   manager.setTarget(0, 4, ""); // Set layer to empty, so the target will become depth Target;
   manager.setTarget(0, 3, "15.0");

   QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();

   EXPECT_DOUBLE_EQ(15.0, targets[0]->z());
   EXPECT_EQ("", targets[0]->surfaceName().toStdString());
}

TEST_F( TestPredictionTargetManager, TestSetSurface )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();

   addSurfaceTargetToManager(manager);
   manager.setTarget(0, 4, "Second Surface");

   QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();

   EXPECT_EQ("Second Surface", targets[0]->surfaceName());
}

TEST_F( TestPredictionTargetManager, TestSwitchDepthToSurfaceTarget )
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();

   addDepthTargetToManager(manager);
   manager.setTarget(0, 4, "Water_Bottom");

   QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargets();

   EXPECT_EQ("Water_Bottom", targets[0]->surfaceName().toStdString());
   EXPECT_DOUBLE_EQ(0.0, targets[0]->z());
}

TEST_F( TestPredictionTargetManager, TestPredictionTargetsWithTimeSeries)
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleDepthTargetsToManager(manager);
   manager.setTargetHasTimeSeries(0, true);
   addSurfaceTargetToManager(manager);
   manager.setTargetHasTimeSeries(2, true);

   QVector<const casaWizard::ua::PredictionTarget*> targets = manager.predictionTargetsIncludingTimeSeries();

   ASSERT_EQ(17, targets.size());

   EXPECT_EQ(1, targets[0]->x());
   EXPECT_EQ(1, targets[7]->x());
   EXPECT_EQ(5, targets[8]->x());
   EXPECT_EQ(2, targets[9]->x());
   EXPECT_EQ(2, targets[16]->x());
}

TEST_F( TestPredictionTargetManager, TestGetIndexInPredictionTargetMatrix)
{
   casaWizard::ua::PredictionTargetManager& manager = targetManager();
   addMultipleDepthTargetsToManager(manager);
   manager.setTargetHasTimeSeries(0, true);
   addSurfaceTargetToManager(manager);
   manager.setTargetHasTimeSeries(2, true);

   const int sizeOfTimeSeries = projectReader_.agesFromMajorSnapshots().size();

   EXPECT_EQ(3, manager.getIndexInPredictionTargetMatrix(0, 3, "Temperature"));
   EXPECT_EQ(sizeOfTimeSeries + 1, manager.getIndexInPredictionTargetMatrix(1, 0, "VRe"));
}

