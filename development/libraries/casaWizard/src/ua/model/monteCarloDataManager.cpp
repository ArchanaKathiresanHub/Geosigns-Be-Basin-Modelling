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
   m_rmse{rmseVector},
   m_influentialParameterMatrix{influentialParameterMatrix},
   m_calibrationTargetMatrix{calibrationTargetMatrix},
   m_predictionTargetMatrix{predictionTargetMatrix},
   m_rmseOptimalRunCase{-9999}
{
}

void MonteCarloDataManager::writeToFile(ScenarioWriter& writer) const
{
   writer.writeValue("MonteCarloDataManagerVersion", 1);
   writer.writeValue("mcmcRMSE", m_rmse);
   writer.writeValue("mcmcInfluentialParameters", m_influentialParameterMatrix);
   writer.writeValue("mcmcCalibrationTargets", m_calibrationTargetMatrix);
   writer.writeValue("mcmcPredictionTargets", m_predictionTargetMatrix);
   writer.writeValue("mcmcRMSEOPtimalRunCase", m_rmseOptimalRunCase);
   writer.writeValue("mcmcInfluentialParameterIdentifiers", m_influentialParameterIdentifiers);
   writer.writeValue("mcmcCalibrationTargetIdentifiers", m_calibrationTargetIdentifiers);
   writer.writeValue("mcmcPredictionTargetIdentifiers", m_predictionTargetIdentifiers);
}

void MonteCarloDataManager::readFromFile(const ScenarioReader& reader)
{
   m_rmse = reader.readVector<double>("mcmcRMSE");
   m_influentialParameterMatrix = reader.readVector<QVector<double>>("mcmcInfluentialParameters");
   m_calibrationTargetMatrix = reader.readVector<QVector<double>>("mcmcCalibrationTargets");
   m_predictionTargetMatrix = reader.readVector<QVector<double>>("mcmcPredictionTargets");
   m_rmseOptimalRunCase = reader.readDouble("mcmcRMSEOPtimalRunCase");
   m_influentialParameterIdentifiers = reader.readVector<QString>("mcmcInfluentialParameterIdentifiers");
   m_calibrationTargetIdentifiers = reader.readVector<QString>("mcmcCalibrationTargetIdentifiers");
   m_predictionTargetIdentifiers = reader.readVector<QString>("mcmcPredictionTargetIdentifiers");
}

void MonteCarloDataManager::clear()
{
   m_rmse.clear();
   m_influentialParameterMatrix.clear();
   m_calibrationTargetMatrix.clear();
   m_predictionTargetMatrix.clear();
   m_rmseOptimalRunCase = -9999;
}

QVector<double> MonteCarloDataManager::rmse() const
{
   return m_rmse;
}

void MonteCarloDataManager::setRmse(const QVector<double>& rmseVector)
{
   m_rmse = rmseVector;
}

QVector<QVector<double>> MonteCarloDataManager::influentialParameterMatrix() const
{
   if (m_rmse.isEmpty())
   {
      return {};
   }
   return m_influentialParameterMatrix;
}

const QVector<QString>& MonteCarloDataManager::influentialParameterIdentifiers() const
{
   return m_influentialParameterIdentifiers;
}

void MonteCarloDataManager::setInfluentialParameterMatrix(const QVector<QVector<double>>& influentialParameterMatrix, const QVector<QString>& parameterIdentifiers)
{
   m_influentialParameterIdentifiers = parameterIdentifiers;
   setInfluentialParameterMatrix(influentialParameterMatrix);
}

void MonteCarloDataManager::setInfluentialParameterMatrix(const QVector<QVector<double>>& influentialParameterMatrix)
{
   m_influentialParameterMatrix = influentialParameterMatrix;
}

QVector<QVector<double>> MonteCarloDataManager::calibrationTargetMatrix() const
{
   if (m_rmse.isEmpty())
   {
      return {};
   }
   return m_calibrationTargetMatrix;
}

const QVector<QString>& MonteCarloDataManager::calibrationTargetIdentifiers() const
{
   return m_calibrationTargetIdentifiers;
}

void MonteCarloDataManager::setCalibrationTargetMatrix(const QVector<QVector<double>>& calibrationTargetMatrix, const QVector<QString>& targetIdentifiers)
{
   m_calibrationTargetIdentifiers = targetIdentifiers;
   setCalibrationTargetMatrix(calibrationTargetMatrix);
}

void MonteCarloDataManager::setCalibrationTargetMatrix(const QVector<QVector<double>>& calibrationTargetMatrix)
{
   m_calibrationTargetMatrix = calibrationTargetMatrix;
}

QVector<QVector<double>> MonteCarloDataManager::predictionTargetMatrix() const
{
   if (m_rmse.isEmpty())
   {
      return {};
   }
   return m_predictionTargetMatrix;
}

const QVector<QString>& MonteCarloDataManager::predictionTargetIdentifiers() const
{
   return m_predictionTargetIdentifiers;
}

void MonteCarloDataManager::setPredictionTargetMatrix(const QVector<QVector<double>>& predictionTargetMatrix, const QVector<QString>& targetIdentifiers)
{
   m_predictionTargetIdentifiers = targetIdentifiers;
   setPredictionTargetMatrix(predictionTargetMatrix);
}

void MonteCarloDataManager::setPredictionTargetMatrix(const QVector<QVector<double>>& predictionTargetMatrix)
{
   m_predictionTargetMatrix = predictionTargetMatrix;
}

double MonteCarloDataManager::rmseOptimalRunCase() const
{
   return m_rmseOptimalRunCase;
}

void MonteCarloDataManager::setRmseOptimalRunCase(double rmseOptimalRunCase)
{
   m_rmseOptimalRunCase = rmseOptimalRunCase;
}

QVector<double> MonteCarloDataManager::getPoint(const int point) const
{
   const int influentialParameters = m_influentialParameterMatrix.size();

   QVector<double> mcmcPoint(influentialParameters, 0);
   for (int i = 0; i < influentialParameters; ++i)
   {
      mcmcPoint[i] = m_influentialParameterMatrix[i][point];
   }

   return mcmcPoint;
}


} // namespace ua

} // namespace casaWizard
