#include "mcDataCreator.h"
#include "model/input/dataFileParser.h"
#include "model/uaScenario.h"

namespace casaWizard
{

namespace ua
{

namespace mcDataCreator
{

void setData(UAScenario& scenario)
{
  // Index of RMSE data in CASA text file for MC data.
  const int indexRMSEmcDataCasaFile = 1;

  if (scenario.workingDirectory().isEmpty())
  {
    throw std::runtime_error("No working directory set");
  }

  if (scenario.mcTextFileName().isEmpty())
  {
    throw std::runtime_error("No MC text file name set");
  }

  DataFileParser<double> parser{scenario.workingDirectory() + "/" + scenario.mcTextFileName()};
  QVector<QVector<double>> mcSampleData = parser.matrixData();

  QVector<QVector<double>> influentialParameterMatrix;
  QVector<QVector<double>> calibrationTargetMatrix;
  QVector<QVector<double>> predictionTargetMatrix;

  const InfluentialParameterManager& ipManager = scenario.influentialParameterManager();
  const int endInfluential = 2 + ipManager.influentialParameters().size();

  const CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
  const int endCalibration = endInfluential + ctManager.amountOfActiveCalibrationTargets();

  const PredictionTargetManager& ptManager = scenario.predictionTargetManager();
  const int endPrediction = endCalibration + ptManager.amountOfPredictionTargetWithTimeSeriesAndProperties();
  for (int i = 2; i < mcSampleData.size(); ++i)
  {
    if (i < endInfluential)
    {
      influentialParameterMatrix.push_back(mcSampleData[i]);
    }
    else if (i < endCalibration)
    {
      calibrationTargetMatrix.push_back(mcSampleData[i]);
    }
    else if (i < endPrediction)
    {
      predictionTargetMatrix.push_back(mcSampleData[i]);
    }
  }

  MonteCarloDataManager& monteCarloDataManager = scenario.monteCarloDataManager();
  monteCarloDataManager.setRmse(mcSampleData[indexRMSEmcDataCasaFile]);
  monteCarloDataManager.setInfluentialParameterMatrix(influentialParameterMatrix);
  monteCarloDataManager.setCalibrationTargetMatrix(calibrationTargetMatrix);
  monteCarloDataManager.setPredictionTargetMatrix(predictionTargetMatrix);
}

} // namespace mcDataCreator

} // namespace ua

} // namespace casaWizard
