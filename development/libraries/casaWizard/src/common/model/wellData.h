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
  unsigned int nCalibrationTargetVars_;
  QVector<QString> calibrationTargetVarsUserName_;
  QVector<QString> calibrationTargetVarsCauldronName_;
  QVector<QString> units_;
  QVector<unsigned int> nDataPerTargetVar_;
  QVector<double> calibrationTargetValues_;
  QVector<double> calibrationTargetStdDeviation_;
  QString metaData_;
  QString wellName_;

  WellData(const double& xCoord = 0.0, const double& yCoord = 0.0, const QVector<double>& depth = {},
           const unsigned int nCalibrationTargetVars = 0, const QVector<QString>& calibrationTargetVarsUserName = {},
           const QVector<unsigned int>& nDataPerTargetVar = {},
           const QVector<double>& calibrationTargetValues = {}, const QVector<double> calibrationTargetStdDeviation = {},
           const QString& metaData = "", const QString& wellName = "",
           const QVector<QString>& units = {}) :
    xCoord_{xCoord},
    yCoord_{yCoord},
    depth_{depth},
    nCalibrationTargetVars_{nCalibrationTargetVars},
    calibrationTargetVarsUserName_{calibrationTargetVarsUserName},
    units_{units},
    nDataPerTargetVar_{nDataPerTargetVar},
    calibrationTargetValues_{calibrationTargetValues},
    calibrationTargetStdDeviation_{calibrationTargetStdDeviation},
    metaData_{metaData},
    wellName_{wellName}
  {
  }
};

} // namespace casaWizard
