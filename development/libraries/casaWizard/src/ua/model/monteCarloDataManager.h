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
  void setInfluentialParameterMatrix(const QVector<QVector<double>>& influentialParameterMatrix);

  QVector<QVector<double>> calibrationTargetMatrix() const;
  void setCalibrationTargetMatrix(const QVector<QVector<double>>& calibrationTargetMatrix);

  QVector<QVector<double>> predictionTargetMatrix() const;
  void setPredictionTargetMatrix(const QVector<QVector<double>>& predictionTargetMatrix);

  double rmseOptimalRunCase() const;
  void setRmseOptimalRunCase(double rmseOptimalRunCase);

  QVector<double> getPoint(const int point) const;

private:
  MonteCarloDataManager(const MonteCarloDataManager&) = delete;
  MonteCarloDataManager& operator=(MonteCarloDataManager) = delete;

  QVector<double> rmse_;
  QVector<QVector<double>> influentialParameterMatrix_;
  QVector<QVector<double>> calibrationTargetMatrix_;
  QVector<QVector<double>> predictionTargetMatrix_;
  double rmseOptimalRunCase_;
};

} // namespace ua

} // namespace casaWizard
