//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "extractWellDataXlsx.h"

#include "model/wellData.h"

#include "xlsxworksheet.h"

#include <string>

namespace casaWizard
{

bool checkValidCell(const QXlsx::Worksheet* sheet, const int row, const int column)
{
  if (!sheet->cellAt(row, column))
  {
    return false;
  }
  if (sheet->cellAt(row,column)->value().toString().isEmpty())
  {
    return false;
  }
  return true;
}


ExtractWellDataXlsx::ExtractWellDataXlsx(const QString& calibrationXlsxFileName) :
  xlsx_{calibrationXlsxFileName},
  nColsCalibrationTarget_{3},
  nextWell_{0},
  numberOfWells_{xlsx_.sheetNames().size()}
{
}

ExtractWellDataXlsx::~ExtractWellDataXlsx()
{
}

void ExtractWellDataXlsx::extractDataNextWell()
{
  // clear previous values to store new well data
  delete wellData_;
  wellData_ = new WellData();
  QXlsx::Worksheet* sheet = dynamic_cast<QXlsx::Worksheet *>(xlsx_.sheet(xlsx_.sheetNames()[nextWell_]));

  readMetaData(sheet);
  readCalibrationData(sheet);

  nextWell_++;
}

void ExtractWellDataXlsx::readMetaData(QXlsx::Worksheet* sheet)
{
  const std::size_t variableRow = 4;
  std::size_t iCol = 1;
  QXlsx::Cell *cell = nullptr;
  bool isCellEmpty = false;
  while (!isCellEmpty)
  {
    cell = sheet->cellAt(variableRow, iCol);
    if (cell)
    {
      if (!cell->value().toString().isEmpty())
      {
        wellData_->calibrationTargetVarsUserName_.push_back(cell->value().toString());
        wellData_->units_.push_back("");
        wellData_->nCalibrationTargetVars_++;
      }
    }
    else
    {
      isCellEmpty = true;
    }

    iCol += nColsCalibrationTarget_;
  }

  // Map target variable names to be compatible with Cauldron
  mapTargetVarNames();

  wellData_->nDataPerTargetVar_.resize(wellData_->nCalibrationTargetVars_);
  std::fill(wellData_->nDataPerTargetVar_.begin(), wellData_->nDataPerTargetVar_.end(), 0);

  wellData_->xCoord_ = sheet->read("B2").toDouble();
  wellData_->yCoord_ = sheet->read("C2").toDouble();
  wellData_->metaData_ = sheet->read("A3").toString();
  wellData_->wellName_ = sheet->read("A2").toString();
}

void ExtractWellDataXlsx::readCalibrationData(QXlsx::Worksheet* sheet)
{
  int column = 1;
  for (std::size_t i_CalTar = 0; i_CalTar < wellData_->nCalibrationTargetVars_; ++i_CalTar)
  {
    int row = 9;

    while (checkValidCell(sheet, row, column))
    {
      wellData_->depth_.append(sheet->cellAt(row, column)->value().toDouble());
      wellData_->nDataPerTargetVar_[i_CalTar] += 1;

      if (sheet->cellAt(row, column+1))
      {
        wellData_->calibrationTargetValues_.append(sheet->cellAt(row, column+1)->value().toDouble());
      }
      if (sheet->cellAt(row, column+2))
      {
        wellData_->calibrationTargetStdDeviation_.append(sheet->cellAt(row, column+2)->value().toDouble());
      }
      else
      {
        wellData_->calibrationTargetStdDeviation_.append(0.0);
      }

      row += 1;
    }
    column += nColsCalibrationTarget_;
  }
}

void ExtractWellDataXlsx::extractMetaDataNextWell()
{
  // clear previous values to store new well data
  delete wellData_;
  wellData_ = new WellData();
  QXlsx::Worksheet* sheet = dynamic_cast<QXlsx::Worksheet *>(xlsx_.sheet(xlsx_.sheetNames()[nextWell_]));

  readMetaData(sheet);

  nextWell_++;
}

bool ExtractWellDataXlsx::hasNextWell() const
{
  return nextWell_ < numberOfWells_;
}

void ExtractWellDataXlsx::resetExtractor()
{
  nextWell_ = 0;
}


} //namespace casaWizard
