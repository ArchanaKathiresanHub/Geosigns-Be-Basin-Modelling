// Class containing Monte Carlo (MC) sampled data, and the methods for setting the data.
#pragma once

#include "model/writable.h"

#include <QVector>

namespace casaWizard
{

namespace ua
{

class MonteCarloDataManager : public Writable
{
public:
  MonteCarloDataManager(const QVector<double>& rmseVector = {},
         const QVector<QVector<double>>& influentialParameterMatrix = {},
         const QVector<QVector<double>>& calibrationTargetMatrix = {},
         const QVector<QVector<double>>& predictionTargetMatrix = {});

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

  QVector<double> rmse() const;
  void setRmse(const QVector<double>& rmse);

  QVector<QVector<double>> influentialParameterMatrix() const;
  const QVector<QString>& influentialParameterIdentifiers() const;
  void setInfluentialParameterMatrix(const QVector<QVector<double>>& influentialParameterMatrix, const QVector<QString>& parameterIdentifiers);
  void setInfluentialParameterMatrix(const QVector<QVector<double>>& influentialParameterMatrix);

  QVector<QVector<double>> calibrationTargetMatrix() const;
  const QVector<QString>& calibrationTargetIdentifiers() const;
  void setCalibrationTargetMatrix(const QVector<QVector<double>>& calibrationTargetMatrix, const QVector<QString>& targetIdentifiers);
  void setCalibrationTargetMatrix(const QVector<QVector<double>>& calibrationTargetMatrix);

  QVector<QVector<double>> predictionTargetMatrix() const;
  const QVector<QString>& predictionTargetIdentifiers() const;
  void setPredictionTargetMatrix(const QVector<QVector<double>>& predictionTargetMatrix, const QVector<QString>& targetIdentifiers);
  void setPredictionTargetMatrix(const QVector<QVector<double>>& predictionTargetMatrix);

  double rmseOptimalRunCase() const;
  void setRmseOptimalRunCase(double rmseOptimalRunCase);

  QVector<double> getPoint(const int point) const;

private:
  MonteCarloDataManager(const MonteCarloDataManager&) = delete;
  MonteCarloDataManager& operator=(MonteCarloDataManager) = delete;

  QVector<double> m_rmse;
  QVector<QVector<double>> m_influentialParameterMatrix;
  QVector<QVector<double>> m_calibrationTargetMatrix;
  QVector<QVector<double>> m_predictionTargetMatrix;
  QVector<QString> m_influentialParameterIdentifiers;
  QVector<QString> m_calibrationTargetIdentifiers;
  QVector<QString> m_predictionTargetIdentifiers;

  double m_rmseOptimalRunCase;
};

} // namespace ua

} // namespace casaWizard
