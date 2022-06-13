#include "model/monteCarloDataManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST( TestMonteCarloData, testWriteToFile )
{
   casaWizard::ua::MonteCarloDataManager mcData;
   mcData.setRmseOptimalRunCase(123);
   mcData.setRmse({1, 2});

   QVector<QVector<double>> ctMatrix = {{11, 12}, {21, 22}};
   QVector<QVector<double>> ipMatrix = {{33, 34}, {43, 44}};
   QVector<QVector<double>> ptMatrix = {{55, 56}, {65, 66}};

   mcData.setCalibrationTargetMatrix(ctMatrix);
   mcData.setInfluentialParameterMatrix(ipMatrix);
   mcData.setPredictionTargetMatrix(ptMatrix);

   {
      casaWizard::ScenarioWriter writer{"monteCarloDataManagerActual.dat"};
      mcData.writeToFile(writer);
      writer.close();
   }

   expectFileEq("monteCarloDataManagerActual.dat", "monteCarloDataManager.dat");

   QVector<QString> ctHeaders = {"cth1","cth2"};
   QVector<QString> ipHeaders = {"iph1","iph2"};
   QVector<QString> ptHeaders = {"pth1","pth2"};

   mcData.setPredictionTargetMatrix(ctMatrix,ctHeaders);
   mcData.setInfluentialParameterMatrix(ipMatrix,ipHeaders);
   mcData.setPredictionTargetMatrix(ptMatrix,ptHeaders);

   {
      casaWizard::ScenarioWriter writer{"monteCarloDataManagerActualHeaders.dat"};
      mcData.writeToFile(writer);
      writer.close();
   }
   expectFileEq("monteCarloDataManagerActualHeaders.dat", "monteCarloDataManagerHeaders.dat");
}

TEST( TestMonteCarloData, testReadFromFile )
{
   casaWizard::ua::MonteCarloDataManager mcData;
   casaWizard::ScenarioReader reader{"monteCarloDataManager.dat"};
   mcData.readFromFile(reader);

   EXPECT_DOUBLE_EQ(mcData.rmseOptimalRunCase(), 123);

   const QVector<double> rmseVector = mcData.rmse();
   ASSERT_EQ(rmseVector.size(), 2);
   EXPECT_DOUBLE_EQ(rmseVector[0], 1);
   EXPECT_DOUBLE_EQ(rmseVector[1], 2);

   const QVector<QVector<double>> ct = mcData.calibrationTargetMatrix();
   ASSERT_EQ(ct.size(),2);
   ASSERT_EQ(ct[0].size(), 2);
   EXPECT_DOUBLE_EQ(ct[0][0], 11);
   EXPECT_DOUBLE_EQ(ct[0][1], 12);
   EXPECT_DOUBLE_EQ(ct[1][0], 21);
   EXPECT_DOUBLE_EQ(ct[1][1], 22);

   const QVector<QVector<double>> ip = mcData.influentialParameterMatrix();
   ASSERT_EQ(ip.size(),2);
   ASSERT_EQ(ip[0].size(), 2);
   EXPECT_DOUBLE_EQ(ip[0][0], 33);
   EXPECT_DOUBLE_EQ(ip[0][1], 34);
   EXPECT_DOUBLE_EQ(ip[1][0], 43);
   EXPECT_DOUBLE_EQ(ip[1][1], 44);

   const QVector<QVector<double>> pt = mcData.predictionTargetMatrix();
   ASSERT_EQ(pt.size(),2);
   ASSERT_EQ(pt[0].size(), 2);
   EXPECT_DOUBLE_EQ(pt[0][0], 55);
   EXPECT_DOUBLE_EQ(pt[0][1], 56);
   EXPECT_DOUBLE_EQ(pt[1][0], 65);
   EXPECT_DOUBLE_EQ(pt[1][1], 66);
}

TEST( TestMonteCarloData, testGetterForEmptyRMSE )
{
   casaWizard::ua::MonteCarloDataManager mcData;
   mcData.setRmse({});

   mcData.setCalibrationTargetMatrix({{1,2},{3,4}});
   mcData.setInfluentialParameterMatrix({{1,2},{3,4}});
   mcData.setPredictionTargetMatrix({{1,2},{3,4}});

   EXPECT_TRUE(mcData.calibrationTargetMatrix().isEmpty());
   EXPECT_TRUE(mcData.influentialParameterMatrix().isEmpty());
   EXPECT_TRUE(mcData.predictionTargetMatrix().isEmpty());
}

TEST( TestMonteCarloData, testGetPoint )
{
   casaWizard::ua::MonteCarloDataManager mcData;
   mcData.setCalibrationTargetMatrix({{10,20},{30,40}});
   mcData.setInfluentialParameterMatrix({{11,21},{31,41}});
   mcData.setPredictionTargetMatrix({{12,22},{32,42}});

   QVector<double> mcmcPoint = mcData.getPoint(0);
   ASSERT_EQ(2, mcmcPoint.size());
   EXPECT_EQ(11, mcmcPoint[0]);
   EXPECT_EQ(31, mcmcPoint[1]);

   QVector<double> mcmcPoint2 = mcData.getPoint(1);
   ASSERT_EQ(2, mcmcPoint2.size());
   EXPECT_EQ(21, mcmcPoint2[0]);
   EXPECT_EQ(41, mcmcPoint2[1]);
}
