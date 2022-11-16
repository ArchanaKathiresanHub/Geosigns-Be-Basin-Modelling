#include "model/uaScenario.h"

#include "model/influentialParameter/topCrustHeatProduction.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "stubProjectReader.h"

#include "expectFileEq.h"

#include <QDir>

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

   writeScenario.setProxyOrder(-1); //should default to "2"
   writeScenario.setProxyKrigingMethod("Local"); //should default to "No"

   //Step is performed in read, thus is needed to ensure that written and read state is equal.
   writeScenario.updateDoeConstantNumberOfDesignPoints(ipManagerWrite.totalNumberOfInfluentialParameters());
   writeScenario.updateStateFileNameDoE();
   {
      casaWizard::ScenarioWriter writer{"scenario.dat"};
      writeScenario.writeToFile(writer);
      writer.close();
   }

   casaWizard::ua::UAScenario readScenario{new casaWizard::StubProjectReader()};
   casaWizard::ScenarioReader reader{"scenario.dat"};
   readScenario.readFromFile(reader);
   {
      casaWizard::ScenarioWriter writer{"scenarioRead.dat"};
      readScenario.writeToFile(writer);
      writer.close();
   }

   expectFileEq("scenario.dat","scenarioRead.dat");

   const casaWizard::ua::InfluentialParameterManager& ipManagerRead = readScenario.influentialParameterManager();

   const QVector<casaWizard::ua::InfluentialParameter*> params = ipManagerRead.influentialParameters();
   ASSERT_EQ(params.size(),1);
   ASSERT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProduction*>(params[0]));
   EXPECT_DOUBLE_EQ(params[0]->arguments().getDoubleArgument(0), 10);
   EXPECT_DOUBLE_EQ(params[0]->arguments().getDoubleArgument(1), 11);

   const casaWizard::ua::PredictionTargetManager& ptManagerRead = readScenario.predictionTargetManager();
   const QVector<const casaWizard::ua::PredictionTarget*> targets = ptManagerRead.predictionTargets();

   EXPECT_EQ(writeScenario.stateFileNameDoE(),readScenario.stateFileNameDoE());

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

   const casaWizard::ua::Proxy& proxyRead = readScenario.proxy();
   EXPECT_EQ(proxyRead.krigingMethod(), "No");
   EXPECT_EQ(proxyRead.order(), 2);
}

// This unit test only covers the additions
TEST( UAScenarioTest, TestWriteReadVersion1 )
{
   casaWizard::ua::UAScenario writeScenario{new casaWizard::StubProjectReader()};
   writeScenario.initializeBaseSubSamplingFactor();// Sets BaseSubSampling factor to 5, due to return value of stubprojectreader
   writeScenario.setSubSamplingFactor(10);

   McmcSettings mcmcSettings;
   mcmcSettings.setNumSamples(600);
   mcmcSettings.setStandardDeviationFactor(2.0);
   writeScenario.setMcmcSettings(mcmcSettings);

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
   ASSERT_EQ(2, mdpManagerRead.numberOfVisiblePoints());

   QVector<QVector<double>> pointsToRun = mdpManagerRead.parameters();
   QVector<double> designPoint0 = pointsToRun[0];
   QVector<double> designPoint1 = pointsToRun[1];

   EXPECT_EQ(600, readScenario.mcmcSettings().nSamples());
   EXPECT_DOUBLE_EQ(2.0, readScenario.mcmcSettings().standardDeviationFactor());

   EXPECT_EQ(10, readScenario.subSamplingFactor());
   EXPECT_EQ(5, readScenario.baseSubSamplingFactor());
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

TEST(UAScenarioTest, TestInit)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   //Base case should be selected by default:
   EXPECT_TRUE(scenario.doeOptionSelectedNames().contains("BaseCase"));
   EXPECT_TRUE(scenario.qcDoeOptionSelectedNames().contains("BaseCase"));
}

TEST(UAScenarioTest, TestClear)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   McmcSettings mcmcSettings;
   mcmcSettings.setNumSamples(600);
   mcmcSettings.setStandardDeviationFactor(2.0);
   scenario.setMcmcSettings(mcmcSettings);

   scenario.clear();

   EXPECT_TRUE(scenario.doeOptionSelectedNames().contains("BaseCase"));
   EXPECT_TRUE(scenario.qcDoeOptionSelectedNames().contains("BaseCase"));
   EXPECT_EQ(500, scenario.mcmcSettings().nSamples());
   EXPECT_DOUBLE_EQ(1.6, scenario.mcmcSettings().standardDeviationFactor());
}

TEST(UAScenarioTest, PredictionTargetDataBestMC)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   PredictionTargetManager& ptManager = scenario.predictionTargetManager();
   ptManager.addDepthTarget(4.0, 5.0, 6.0, {"VRe"}, 0.0);

   MonteCarloDataManager& mcManager = scenario.monteCarloDataManager();
   QString identifierVRe = ptManager.predictionTargets()[0]->identifier("VRe");
   mcManager.setPredictionTargetMatrix({{55, 56, 57}},{identifierVRe});

   //With no calibration target, the best MC cannot be determined. Function should return empty vector.
   EXPECT_EQ(scenario.predictionTargetDataBestMC().size(),0);

   CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
   ctManager.addWell("",0,0);
   ctManager.addCalibrationTarget("","p",0,0,0);
   mcManager.setRmse({1.0,2.0,3.0});

   //With calibration targets, the best MC should be the first entry:
   QVector<double> bestMc = scenario.predictionTargetDataBestMC();
   EXPECT_EQ(bestMc.size(),1);
   EXPECT_DOUBLE_EQ(bestMc[0],55);
}

TEST(UAScenarioTest, setOptimalValuesTargetQCs)
{
   UAScenario scenario(new casaWizard::StubProjectReader());

   QVector<TargetQC> targetQCs;
   const int nTargets = 3;

   for (int i = 0; i < nTargets; i++)
   {
      const QString identifier = "id" + QString::number(i);
      targetQCs.push_back(TargetQC(0,"","",identifier,false,0,0,0,0,0,{},{},0));
   }

   scenario.setTargetQCs(targetQCs);

   const QVector<double> values{1,2,3};
   const QVector<QString> names{"id1","id2","id0"};

   //Optimal values are matched to the targets based on the identifiers.
   scenario.setOptimalValuesTargetQCs(values, names);

   targetQCs = scenario.targetQCs();

   EXPECT_DOUBLE_EQ(targetQCs[0].yOptimalSim(),3);
   EXPECT_DOUBLE_EQ(targetQCs[1].yOptimalSim(),1);
   EXPECT_DOUBLE_EQ(targetQCs[2].yOptimalSim(),2);
}

TEST(UAScenarioTest, copyToIterationDir)
{
   UAScenario scenario(new casaWizard::StubProjectReader());

   QString wDir = "wDir";
   QString iDir  ="Iteration_1";
   QString runLocation = "CaseSet";

   //cleanup (in case folder still present)
   QString wDirPath = QDir::currentPath() + "/" + wDir;
   QDir dirToRemove(wDirPath);
   dirToRemove.removeRecursively();

   //Make file location:
   QString testDirPath = QDir::currentPath() + "/" + wDir + "/" + runLocation + "/" + iDir;
   QDir tmpTestDir;
   tmpTestDir.mkpath(testDirPath);

   scenario.setWorkingDirectory(wDirPath);
   scenario.setRunLocation(runLocation);

   QString fileName = "testFile";
   QString filePath = QDir::currentPath() + "/" + wDir + "/" + fileName;

   //Create empty file:
   QFile file(filePath);
   file.open(QFile::OpenModeFlag::WriteOnly);
   file.close();
   scenario.updateIterationDir();
   scenario.copyToIterationDir(fileName);

   //File should be copied to testDirpath. Old file should be removed:
   EXPECT_TRUE(QFile::exists(testDirPath + "/" + fileName));
   EXPECT_FALSE(QFile::exists(filePath));

   file.open(QFile::OpenModeFlag::WriteOnly);
   QTextStream out(&file);
   QString writeString("test");
   out << writeString;
   file.close();

   scenario.copyToIterationDir(fileName);

   //File should be replaced by new non-empty file:
   QFile file2(testDirPath + "/" + fileName);
   file2.open(QFile::OpenModeFlag::ReadOnly);
   QTextStream out2(&file2);
   QString content = out2.readLine();
   EXPECT_EQ(content,writeString);

   //cleanup
   dirToRemove.removeRecursively();
}

TEST(UAScenarioTest, stageStates)
{
   UAScenario scenario(new casaWizard::StubProjectReader());

   //Default false:
   EXPECT_FALSE(scenario.isStageComplete(StageTypesUA::doe));
   EXPECT_FALSE(scenario.isStageComplete(StageTypesUA::responseSurfaces));
   EXPECT_FALSE(scenario.isStageComplete(StageTypesUA::mcmc));

   //Default true:
   EXPECT_TRUE(scenario.isStageUpToDate(StageTypesUA::doe));
   EXPECT_TRUE(scenario.isStageUpToDate(StageTypesUA::responseSurfaces));
   EXPECT_TRUE(scenario.isStageUpToDate(StageTypesUA::mcmc));

   scenario.setStageComplete(StageTypesUA::responseSurfaces,true);
   EXPECT_TRUE(scenario.isStageComplete(StageTypesUA::responseSurfaces));

   scenario.setStageUpToDate(StageTypesUA::doe,false);
   EXPECT_FALSE(scenario.isStageUpToDate(StageTypesUA::doe));
}

TEST(UAScenarioTest, updateStateFileNameDoE)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   QString stateFileName = scenario.updateStateFileNameDoE();
   QString sameStateFileName = scenario.stateFileNameDoE();
   EXPECT_EQ(stateFileName,sameStateFileName);

   QString otherStateFileName = scenario.updateStateFileNameDoE();
   EXPECT_NE(stateFileName,otherStateFileName);
}

TEST(UAScenarioTest, simStatesTextFileName)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   QString name = scenario.simStatesTextFileName();

   //Changing the name should be no problem for the application, as long as the name is not empty. Therefore we only test for that.
   EXPECT_TRUE(name != "");
}

TEST(UAScenarioTest, iterationDirExists)
{
   UAScenario scenario(new casaWizard::StubProjectReader());
   scenario.setWorkingDirectory(QDir::currentPath());
   EXPECT_FALSE(scenario.iterationDirExists());

   QDir tmpTestDir(QDir::currentPath() + "/CaseSet");
   tmpTestDir.removeRecursively();
   tmpTestDir.mkpath(QDir::currentPath() + "/CaseSet/Iteration1");
   scenario.updateIterationDir();
   EXPECT_TRUE(scenario.iterationDirExists());
   tmpTestDir.removeRecursively();
}

