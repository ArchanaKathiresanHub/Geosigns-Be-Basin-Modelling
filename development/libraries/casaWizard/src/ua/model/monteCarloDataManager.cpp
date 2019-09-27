#include "monteCarloDataManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace ua
{

MonteCarloDataManager::MonteCarloDataManager(const QVector<double>& rmseVector,
                                             const QVector<QVector<double>>& influentialParameterMatrix,
                                             const QVector<QVector<double>>& calibrationTargetMatrix,
                                             const QVector<QVector<double>>& predictionTargetMatrix) :
  rmse_{rmseVector},
  influentialParameterMatrix_{influentialParameterMatrix},
  calibrationTargetMatrix_{calibrationTargetMatrix},
  predictionTargetMatrix_{predictionTargetMatrix},
  rmseOptimalRunCase_{-9999}
{
}

void MonteCarloDataManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("MonteCarloDataManagerVersion", 0);
  writer.writeValue("mcmcRMSE", rmse_);
  writer.writeValue("mcmcInfluentialParameters", influentialParameterMatrix_);
  writer.writeValue("mcmcCalibrationTargets", calibrationTargetMatrix_);
  writer.writeValue("mcmcPredictionTargets", predictionTargetMatrix_);
  writer.writeValue("mcmcRMSEOPtimalRunCase", rmseOptimalRunCase_);
}

void MonteCarloDataManager::readFromFile(const ScenarioReader& reader)
{
  rmse_ = reader.readVector<double>("mcmcRMSE");
  influentialParameterMatrix_ = reader.readVector<QVector<double>>("mcmcInfluentialParameters");
  calibrationTargetMatrix_ = reader.readVector<QVector<double>>("mcmcCalibrationTargets");
  predictionTargetMatrix_ = reader.readVector<QVector<double>>("mcmcPredictionTargets");
  rmseOptimalRunCase_ = reader.readDouble("mcmcRMSEOPtimalRunCase");
}

void MonteCarloDataManager::clear()
{
  rmse_.clear();
  influentialParameterMatrix_.clear();
  calibrationTargetMatrix_.clear();
  predictionTargetMatrix_.clear();
  rmseOptimalRunCase_ = -9999;
}

QVector<double> MonteCarloDataManager::rmse() const
{
  return rmse_;
}

void MonteCarloDataManager::setRmse(const QVector<double>& rmseVector)
{
  rmse_ = rmseVector;
}

QVector<QVector<double>> MonteCarloDataManager::influentialParameterMatrix() const
{
  if (rmse_.isEmpty())
  {
    return {};
  }
  return influentialParameterMatrix_;
}

void MonteCarloDataManager::setInfluentialParameterMatrix(const QVector<QVector<double>>& influentialParameterMatrix)
{
  influentialParameterMatrix_ = influentialParameterMatrix;
}

QVector<QVector<double>> MonteCarloDataManager::calibrationTargetMatrix() const
{
  if (rmse_.isEmpty())
  {
    return {};
  }
  return calibrationTargetMatrix_;
}

void MonteCarloDataManager::setCalibrationTargetMatrix(const QVector<QVector<double>>& calibrationTargetMatrix)
{
  calibrationTargetMatrix_ = calibrationTargetMatrix;
}

QVector<QVector<double>> MonteCarloDataManager::predictionTargetMatrix() const
{
  if (rmse_.isEmpty())
  {
    return {};
  }
  return predictionTargetMatrix_;
}

void MonteCarloDataManager::setPredictionTargetMatrix(const QVector<QVector<double>>& predictionTargetMatrix)
{
  predictionTargetMatrix_ = predictionTargetMatrix;
}

double MonteCarloDataManager::rmseOptimalRunCase() const
{
  return rmseOptimalRunCase_;
}

void MonteCarloDataManager::setRmseOptimalRunCase(double rmseOptimalRunCase)
{
  rmseOptimalRunCase_ = rmseOptimalRunCase;
}

QVector<double> MonteCarloDataManager::getPoint(const int point) const
{
  const int influentialParameters = influentialParameterMatrix_.size();

  QVector<double> mcmcPoint(influentialParameters, 0);
  for (int i = 0; i < influentialParameters; ++i)
  {
    mcmcPoint[i] = influentialParameterMatrix_[i][point];
  }

  return mcmcPoint;
}


} // namespace ua

} // namespace casaWizard
