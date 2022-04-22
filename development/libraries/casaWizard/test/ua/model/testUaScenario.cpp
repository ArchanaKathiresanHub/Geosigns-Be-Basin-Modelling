#include "model/uaScenario.h"

#include "model/influentialParameter/topCrustHeatProduction.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "stubProjectReader.h"

#include <gtest/gtest.h>

using namespace casaWizard;
using namespace ua;

TEST( UAScenarioTest, TestWriteReadVersion0 )
{
  casaWizard::ua::UAScenario writeScenario{new casaWizard::StubProjectReader()};

  casaWizard::ua::InfluentialParameterManager& ipManagerWrite = writeScenario.influentialParameterManager();
  ipManagerWrite.add(0);
  ipManagerWrite.setArguments(0, {10, 11}, {});

  casaWizard::ua::PredictionTargetManager& ptManagerWrite = writeScenario.predictionTargetManager();
  ptManagerWrite.addDepthTarget(1.0, 2.0, 3.0, {"Temperature"});
  ptManagerWrite.addDepthTarget(4.0, 5.0, 6.0, {"VRe"});

  casaWizard::ua::MonteCarloDataManager& mcManagerWrite = writeScenario.monteCarloDataManager();
  mcManagerWrite.setRmse({1, 2});
  mcManagerWrite.setCalibrationTargetMatrix({{11, 12}, {21, 22}});
  mcManagerWrite.setInfluentialParameterMatrix({{33, 34}, {43, 44}});
  mcManagerWrite.setPredictionTargetMatrix({{55, 56}, {65, 66}});

  casaWizard::ScenarioWriter writer{"scenario.dat"};
  writeScenario.writeToFile(writer);
  writer.close();

  casaWizard::ua::UAScenario readScenario{new casaWizard::StubProjectReader()};
  casaWizard::ScenarioReader reader{"scenario.dat"};
  readScenario.readFromFile(reader);

  const casaWizard::ua::InfluentialParameterManager& ipManagerRead = readScenario.influentialParameterManager();

  const QVector<casaWizard::ua::InfluentialParameter*> params = ipManagerRead.influentialParameters();
  ASSERT_EQ(params.size(),1);
  ASSERT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProduction*>(params[0]));
  EXPECT_DOUBLE_EQ(params[0]->arguments().getDoubleArgument(0), 10);
  EXPECT_DOUBLE_EQ(params[0]->arguments().getDoubleArgument(1), 11);

  const casaWizard::ua::PredictionTargetManager& ptManagerRead = readScenario.predictionTargetManager();
  const QVector<const casaWizard::ua::PredictionTarget*> targets = ptManagerRead.predictionTargets();

  ASSERT_EQ(targets.size(), 2);

  EXPECT_DOUBLE_EQ(targets[0]->x(), 1);
  EXPECT_DOUBLE_EQ(targets[0]->y(), 2);
  EXPECT_DOUBLE_EQ(targets[0]->z(), 3);

  EXPECT_DOUBLE_EQ(targets[1]->x(), 4);
  EXPECT_DOUBLE_EQ(targets[1]->y(), 5);
  EXPECT_DOUBLE_EQ(targets[1]->z(), 6);

  const casaWizard::ua::MonteCarloDataManager& mcManagerRead = readScenario.monteCarloDataManager();

  const QVector<double> rmseVector = mcManagerRead.rmse();
  ASSERT_EQ(rmseVector.size(), 2);
  EXPECT_DOUBLE_EQ(rmseVector[0], 1);
  EXPECT_DOUBLE_EQ(rmseVector[1], 2);

  const QVector<QVector<double>>& ct = mcManagerRead.calibrationTargetMatrix();
  ASSERT_EQ(ct.size(),2);
  ASSERT_EQ(ct[0].size(), 2);
  EXPECT_DOUBLE_EQ(ct[0][0], 11);
  EXPECT_DOUBLE_EQ(ct[0][1], 12);
  EXPECT_DOUBLE_EQ(ct[1][0], 21);
  EXPECT_DOUBLE_EQ(ct[1][1], 22);

  const QVector<QVector<double>>& ip = mcManagerRead.influentialParameterMatrix();
  ASSERT_EQ(ip.size(),2);
  ASSERT_EQ(ip[0].size(), 2);
  EXPECT_DOUBLE_EQ(ip[0][0], 33);
  EXPECT_DOUBLE_EQ(ip[0][1], 34);
  EXPECT_DOUBLE_EQ(ip[1][0], 43);
  EXPECT_DOUBLE_EQ(ip[1][1], 44);

  const QVector<QVector<double>>& pt = mcManagerRead.predictionTargetMatrix();
  ASSERT_EQ(pt.size(),2);
  ASSERT_EQ(pt[0].size(), 2);
  EXPECT_DOUBLE_EQ(pt[0][0], 55);
  EXPECT_DOUBLE_EQ(pt[0][1], 56);
  EXPECT_DOUBLE_EQ(pt[1][0], 65);
  EXPECT_DOUBLE_EQ(pt[1][1], 66);
}

// This unit test only covers the additions
TEST( UAScenarioTest, TestWriteReadVersion1 )
{
  casaWizard::ua::UAScenario writeScenario{new casaWizard::StubProjectReader()};

  casaWizard::ua::ManualDesignPointManager& mdpManagerWrite = writeScenario.manualDesignPointManager();
  mdpManagerWrite.addInfluentialParameter(3);
  mdpManagerWrite.addDesignPoint({11, 12, 13});
  mdpManagerWrite.addDesignPoint({21, 22, 23});

  casaWizard::ScenarioWriter writer{"scenario1.dat"};
  writeScenario.writeToFile(writer);
  writer.close();

  casaWizard::ua::UAScenario readScenario{new casaWizard::StubProjectReader()};
  casaWizard::ScenarioReader reader{"scenario1.dat"};
  readScenario.readFromFile(reader);
  const casaWizard::ua::ManualDesignPointManager& mdpManagerRead = readScenario.manualDesignPointManager();

  ASSERT_EQ(3, mdpManagerRead.numberOfParameters());
  ASSERT_EQ(2, mdpManagerRead.numberOfPoints());

  QVector<double> designPoint0 = mdpManagerRead.getDesignPoint(0);
  QVector<double> designPoint1 = mdpManagerRead.getDesignPoint(1);

  EXPECT_EQ(11, designPoint0[0]);
  EXPECT_EQ(12, designPoint0[1]);
  EXPECT_EQ(13, designPoint0[2]);
  EXPECT_EQ(21, designPoint1[0]);
  EXPECT_EQ(22, designPoint1[1]);
  EXPECT_EQ(23, designPoint1[2]);
}

void initializeScenarioWithPredictionTargets(UAScenario& scenario)
{
   MonteCarloDataManager& mcManager = scenario.monteCarloDataManager();
   PredictionTargetManager& predictionTargetManager = scenario.predictionTargetManager();
   predictionTargetManager.addDepthTarget(0, 0, 0, {"VRe"}); // First target without time series
   predictionTargetManager.addDepthTarget(0, 0, 0, {"Temperature", "VRe"}); // Second target with time series
   predictionTargetManager.setTargetHasTimeSeries(1, true);

   QVector<QVector<double>> predMatrix;

   // Add data for first target
   predMatrix.push_back({0.4, 0.8, 0.5});

   // Add matrix data for second target with time series
   for (int i = 0; i < scenario.projectReader().agesFromMajorSnapshots().size(); i++)
   {
      predMatrix.push_back({i + 0.1, i - 0.1, i * 1.0}); // add one entry for Temperature for every time-step
      predMatrix.push_back({0.01* (i + 0.1), 0.01*(i - 0.1), i * 0.01}); // add one entry for VRe for every time-step
   }

   mcManager.setPredictionTargetMatrix(predMatrix);
   mcManager.setRmse({1.0}); // Putting Rmse to non-empty, to enable manager to return predictionTargetMatrix
}

TEST(UAScenarioTest, TestGetTimeSeriesData)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   initializeScenarioWithPredictionTargets(scenario);

   QVector<double> snapshotAges;
   QMap<QString,QVector<double>> bestMatchedValuesPerProperty; // Initialize one best matched values vector per property
   QMap<QString,QVector<QVector<double>>> currentPredTargetMatrixPerProperty; // Initialize one prediction matrix per property, indexed in a map
   scenario.obtainTimeSeriesMonteCarloData(1, snapshotAges, bestMatchedValuesPerProperty, currentPredTargetMatrixPerProperty);

   EXPECT_TRUE(bestMatchedValuesPerProperty["Non-existentProperty"].empty());
   ASSERT_EQ(snapshotAges.size(), bestMatchedValuesPerProperty["VRe"].size());
   ASSERT_EQ(snapshotAges.size(), bestMatchedValuesPerProperty["Temperature"].size());
   EXPECT_DOUBLE_EQ(1.1, bestMatchedValuesPerProperty["Temperature"][1]);
   EXPECT_DOUBLE_EQ(0.021, bestMatchedValuesPerProperty["VRe"][2]);

   EXPECT_TRUE(currentPredTargetMatrixPerProperty["Non-existentProperty"].empty());
   ASSERT_EQ(snapshotAges.size(), currentPredTargetMatrixPerProperty["VRe"].size());
   ASSERT_EQ(snapshotAges.size(), currentPredTargetMatrixPerProperty["Temperature"].size());
   EXPECT_DOUBLE_EQ(0.9, currentPredTargetMatrixPerProperty["Temperature"][1][1]);
   EXPECT_DOUBLE_EQ(0.02, currentPredTargetMatrixPerProperty["VRe"][2][2]);
}

TEST(UAScenarioTest, TestGetDataAtTimeStep)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   initializeScenarioWithPredictionTargets(scenario);

   QVector<QVector<double>> data;
   scenario.obtainMonteCarloDataForTimeStep(0, 0, data);

   EXPECT_TRUE(data[0].empty()); // No Temperature data in the first target
   EXPECT_EQ(3, data[1].size());
   EXPECT_DOUBLE_EQ(0.8, data[1][1]); // data was initialized as predMatrix.push_back({0.4, 0.8, 0.5});

   data.clear();
   scenario.obtainMonteCarloDataForTimeStep(1, 4, data);

   EXPECT_EQ(3, data[0].size());
   EXPECT_EQ(3, data[1].size());

   EXPECT_DOUBLE_EQ(4.1, data[0][0]);
   EXPECT_DOUBLE_EQ(0.039, data[1][1]);
}
