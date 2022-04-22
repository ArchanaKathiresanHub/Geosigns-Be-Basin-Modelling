#include "model/input/mcDataCreator.h"
#include "model/uaScenario.h"

#include "stubProjectReader.h"
#include <gtest/gtest.h>

TEST( MCDataCreatorTest, testSetData )
{
  casaWizard::ua::UAScenario scenario{new casaWizard::StubProjectReader()};
  const QString mcTextFileName{"testCasaV20161104MCdata.dat"};
  scenario.setMCtextFileName(mcTextFileName);
  scenario.setWorkingDirectory(".");

  // Remove (any) default influential parameters, then add new ones.
  casaWizard::ua::InfluentialParameterManager& ipManager = scenario.influentialParameterManager();
  ipManager.clear();
  ipManager.add();

  // Add a calibration target.
  casaWizard::CalibrationTargetManager& ctManager{scenario.calibrationTargetManager()};
  const int wellId = ctManager.addWell("test", 0.0, 0.0);
  ctManager.addCalibrationTarget("calibrationTarget0", "T", wellId, 100.0, 18.0);

  // Add a prediction target.
  casaWizard::ua::PredictionTargetManager& ptManager{scenario.predictionTargetManager()};
  ptManager.addDepthTarget(1000.0, 1500.0, 100.0, {"T"}); // With time series
  ptManager.setTargetHasTimeSeries(0, true);
  ptManager.addDepthTarget(1000.0, 1500.0, 400.0, {"V"}); // Without time series (present day only)

  const QVector<double> rmseExpected = {0.1, 0.11, 0.6};
  const QVector<double> mcSampledInfluentialParameterExpected = {10.1, 20.5, 5.4};
  const QVector<double> mcSampledCalibrationTargetExpected = {0.1, 0.15, 0.2};

  casaWizard::ua::mcDataCreator::setData(scenario);

  const casaWizard::ua::MonteCarloDataManager& mcManager{scenario.monteCarloDataManager()};
  const QVector<double> rmseActual = mcManager.rmse();
  const QVector<double> mcSampledInfluentialParameterActual = mcManager.influentialParameterMatrix()[0];
  const QVector<double> mcSampledCalibrationTargetActual = mcManager.calibrationTargetMatrix()[0];
  const QVector<QVector<double>>& ptargets = mcManager.predictionTargetMatrix();
  const QVector<QVector<double>> mcSampledPredictionTargetActual0 = ptargets.mid(ptManager.indexCumulativePredictionTarget(0)
                                                                                 , ptManager.sizeOfPredictionTargetWithTimeSeries(0));
  const QVector<QVector<double>> mcSampledPredictionTargetActual1 = ptargets.mid(ptManager.indexCumulativePredictionTarget(1)
                                                                                 , ptManager.sizeOfPredictionTargetWithTimeSeries(1));

  const QVector<QVector<double>> mcSampledPredictionTargetExpected0 =
  {{19.0, 18.1, 22.3},
   {21.0, 16.0, 17.2},
   {17.0, 14.0, 15.0},
   {22.0, 21.1, 22.1},
   {19.5, 18.4, 17.4},
   {21.1, 23.3, 24.1},
   {18.8, 16.9, 17.5},
   {15.5, 19.1,  20.1}};

  const QVector<QVector<double>> mcSampledPredictionTargetExpected1 =
  {{19.1, 18.2, 17.8}};

  for (int i = 0; i < 3; ++i)
  {
    EXPECT_DOUBLE_EQ(rmseActual[i], rmseExpected[i])
        << "RMSE data did not match the expected value at index [" << i << "]";

    EXPECT_DOUBLE_EQ(mcSampledInfluentialParameterActual[i], mcSampledInfluentialParameterExpected[i])
        << "MC-sampled influential parameter did not match the expected value at index [" << i << "]";

    EXPECT_DOUBLE_EQ(mcSampledCalibrationTargetActual[i], mcSampledCalibrationTargetExpected[i])
        << "MC-sampled calibration target did not match the expected value at index [" << i << "]";

    for (int j = 0; j < mcSampledPredictionTargetExpected0.size(); ++j)
    {
      EXPECT_DOUBLE_EQ(mcSampledPredictionTargetActual0[j][i], mcSampledPredictionTargetExpected0[j][i])
          << "MC-sampled prediction target did not match the expected value at index [" << j << ", " << i << "]";
    }

    for (int j = 0; j < mcSampledPredictionTargetActual1.size(); ++j)
    {
      EXPECT_DOUBLE_EQ(mcSampledPredictionTargetActual1[j][i], mcSampledPredictionTargetExpected1[j][i])
          << "MC-sampled prediction target did not match the expected value at index [" << j << ", " << i << "]";
    }

    ++i;
  }
}
