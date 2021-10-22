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
#include "extractWellData.h"

#include <QString>
#include <QVector>

#include <memory>

namespace casaWizard
{

class ExtractWellDataXlsx : public ExtractWellData
{
public:
  ExtractWellDataXlsx(const QString& xlsxFileName = "");
  ~ExtractWellDataXlsx();

  void extractDataNextWell() final;
  void extractMetaDataNextWell() final;
  bool hasNextWell() const final;
  void resetExtractor() final;

private:
  QXlsx::Document xlsx_;
  std::size_t nColsCalibrationTarget_;
  QVector<QString> wellNames_;
  int nextWell_;
  int numberOfWells_;
  void readMetaData(QXlsx::Worksheet* sheet);
  void readCalibrationData(QXlsx::Worksheet* sheet);
};

} // namespace casaWizard
