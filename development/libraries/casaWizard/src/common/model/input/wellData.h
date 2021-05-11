// Data helper struct for reading Excel calibration data files
#pragma once

#include <QVector>

namespace casaWizard
{

struct WellData
{
  double xCoord_;
  double yCoord_;
  QVector<double> depth_;
  std::size_t nCalibrationTargetVars_;
  QVector<QString> calibrationTargetVarsUserName_;
  QVector<QString> calibrationTargetVarsCauldronName_;
  QVector<std::size_t> nDataPerTargetVar_;
  QVector<double> calibrationTargetValues_;
  QVector<double> calibrationTargetStdDeviation_;

  WellData(const double& xCoord = 0.0, const double& yCoord = 0.0, const QVector<double>& depth = {}, const std::size_t nCalibrationTargetVars = 0, const QVector<QString>& calibrationTargetVarsUserName = {}, const QVector<std::size_t>& nDataPerTargetVar = {}, const QVector<double>& calibrationTargetValues = {}, const QVector<double> calibrationTargetStdDeviation = {}) :
    xCoord_{xCoord},
    yCoord_{yCoord},
    depth_{depth},
    nCalibrationTargetVars_{nCalibrationTargetVars},
    calibrationTargetVarsUserName_{calibrationTargetVarsUserName},
    nDataPerTargetVar_{nDataPerTargetVar},
    calibrationTargetValues_{calibrationTargetValues},
    calibrationTargetStdDeviation_{calibrationTargetStdDeviation}
  {
  }
};

} // namespace casaWizard
