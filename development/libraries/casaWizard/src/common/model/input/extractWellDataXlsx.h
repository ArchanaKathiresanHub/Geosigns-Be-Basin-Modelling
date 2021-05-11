// Reader for an Excel data file with calibration targets order by well and variable type
#pragma once

#include "wellData.h"
#include "xlsxdocument.h"
#include <memory>

namespace casaWizard
{

struct WellData;

class ExtractWellDataXlsx
{
public:
  ExtractWellDataXlsx(const QString& xlsxFileName = "");
  void extractWellData(const QString& wellName);

  void setWellNames();
  const QVector<QString>& wellNames() const;
  double xCoord() const;
  double yCoord() const;
  QVector<double> depth() const;
  std::size_t nCalibrationTargetVars() const;
  QVector<QString> calibrationTargetVarsUserName() const;
  QVector<QString> calibrationTargetVarsCauldronName() const;
  QVector<std::size_t> nDataPerTargetVar() const;
  QVector<double> calibrationTargetValues() const;
  QVector<double> calibrationTargetStdDeviation() const;

  QMap<QString, QString> getCalibrationTargetVariableMaps() const;

private:
  void setCalibrationTargetVariableMaps();
  void mapTargetVarNames();

  QVector<QString> wellNames_;
  QXlsx::Document xlsx_;
  std::size_t nColsCalibrationTarget_;
  QMap<QString, QString> calibrationTargetVariableMaps_;
  std::unique_ptr<WellData> wellData_;
};

} // namespace casaWizard
