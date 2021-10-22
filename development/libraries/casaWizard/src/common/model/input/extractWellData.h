//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QMap>
#include <QString>
#include <QVector>

#include <memory>

namespace casaWizard
{

struct WellData;
struct ImportOptions;

class ExtractWellData
{
public:
  ExtractWellData();
  virtual ~ExtractWellData();

  virtual void extractDataNextWell() = 0;
  virtual void extractMetaDataNextWell() = 0;
  virtual bool hasNextWell() const = 0;
  virtual void resetExtractor() = 0;

  double xCoord() const;
  double yCoord() const;
  const QString& metaData() const;
  const QString& wellName() const;

  QVector<double> depth() const;
  unsigned int nCalibrationTargetVars() const;
  QVector<QString> calibrationTargetVarsUserName() const;
  QVector<QString> calibrationTargetVarsCauldronName() const;
  QVector<unsigned int> nDataPerTargetVar() const;
  QVector<double> calibrationTargetValues() const;
  QVector<double> calibrationTargetStdDeviation() const;
  QMap<QString, QString> calibrationTargetVariableMaps() const;
  QVector<QString> units() const;

protected:
  void mapTargetVarNames();

  WellData* wellData_;
  QMap<QString, QString> calibrationTargetVariableMaps_;

private:
  void setCalibrationTargetVariableMaps();
};

} // namespace casaWizard
