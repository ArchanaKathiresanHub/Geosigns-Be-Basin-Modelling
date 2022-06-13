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

   QString filePath = scenario.workingDirectory() + "/" + scenario.mcTextFileName();
   QVector<QString> columnNames;
   QVector<QVector<double>> mcSampleData = DataFileParser<double>::parseFileWithHeaderColDominant(filePath,columnNames);

   QVector<QVector<double>> influentialParameterMatrix;
   QVector<QVector<double>> calibrationTargetMatrix;
   QVector<QVector<double>> predictionTargetMatrix;

   QVector<QString> influentialParameterIdentifiers;
   QVector<QString> calibrationTargetIdentifiers;
   QVector<QString> predictionTargetIdentifiers;

   const InfluentialParameterManager& ipManager = scenario.influentialParameterManager();
   const int endInfluential = 2 + ipManager.influentialParameters().size();

   const CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
   const int endCalibration = endInfluential + ctManager.amountOfActiveCalibrationTargets();

   const PredictionTargetManager& ptManager = scenario.predictionTargetManager();
   const int endPrediction = endCalibration + ptManager.amountOfPredictionTargetWithTimeSeriesAndProperties();

   if (endPrediction != mcSampleData.size())
   {
      throw std::runtime_error("Mismatch in targets and MCMC output size.");
   }

   influentialParameterMatrix = mcSampleData.mid(2,endInfluential-2);
   influentialParameterIdentifiers = columnNames.mid(2,endInfluential-2);

   calibrationTargetMatrix = mcSampleData.mid(endInfluential,endCalibration-endInfluential);
   calibrationTargetIdentifiers = columnNames.mid(endInfluential,endCalibration-endInfluential);

   predictionTargetMatrix = mcSampleData.mid(endCalibration,endPrediction-endCalibration);
   predictionTargetIdentifiers = columnNames.mid(endCalibration,endPrediction-endCalibration);

   MonteCarloDataManager& monteCarloDataManager = scenario.monteCarloDataManager();
   monteCarloDataManager.setRmse(mcSampleData[indexRMSEmcDataCasaFile]);
   monteCarloDataManager.setInfluentialParameterMatrix(influentialParameterMatrix,influentialParameterIdentifiers);
   monteCarloDataManager.setCalibrationTargetMatrix(calibrationTargetMatrix,calibrationTargetIdentifiers);
   monteCarloDataManager.setPredictionTargetMatrix(predictionTargetMatrix,predictionTargetIdentifiers);
}

} // namespace mcDataCreator

} // namespace ua

} // namespace casaWizard
