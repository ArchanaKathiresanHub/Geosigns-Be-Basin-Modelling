//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


// Reader for an Excel data file with calibration targets order by well and variable type
#pragma once

#include "xlsxdocument.h"

#include <QString>
#include <QVector>

#include <memory>

namespace casaWizard
{

struct WellData;

class ExtractWellDataXlsx
{
public:
  ExtractWellDataXlsx(const QString& xlsxFileName = "");
  ~ExtractWellDataXlsx();

  void extractWellData(const QString& wellName);

  void setWellNames();
  const QVector<QString>& wellNames() const;
  double xCoord() const;
  double yCoord() const;
  const QString& metaData() const;
  QVector<double> depth() const;
  unsigned int nCalibrationTargetVars() const;
  QVector<QString> calibrationTargetVarsUserName() const;
  QVector<QString> calibrationTargetVarsCauldronName() const;
  QVector<unsigned int> nDataPerTargetVar() const;
  QVector<double> calibrationTargetValues() const;
  QVector<double> calibrationTargetStdDeviation() const;

  QMap<QString, QString> calibrationTargetVariableMaps() const;

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
