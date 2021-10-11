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
  wellNames_{},
  xlsx_{calibrationXlsxFileName},
  nColsCalibrationTarget_{3},
  calibrationTargetVariableMaps_{},
  wellData_{new WellData{}}
{
  setCalibrationTargetVariableMaps();
  setWellNames();
}

ExtractWellDataXlsx::~ExtractWellDataXlsx()
{
}

void ExtractWellDataXlsx::extractWellData(const QString& wellName)
{
  // clear previous values to store new well data
  wellData_.reset(new WellData{});

  int i_sheet = -1;
  for (auto ii = 0; ii < xlsx_.sheetNames().size(); ++ii)
  {
    if (wellName == wellNames_[ii])
      i_sheet = ii;
  }

  if (i_sheet == -1)
  {
    throw std::out_of_range("Specified well name does not match any of the ones in the Excel file.");
  }

  QXlsx::Worksheet* sheet = dynamic_cast<QXlsx::Worksheet *>(xlsx_.sheet(xlsx_.sheetNames()[i_sheet]));

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

void ExtractWellDataXlsx::setCalibrationTargetVariableMaps()
{
  calibrationTargetVariableMaps_.insert("TwoWayTime", "TwoWayTime");
  calibrationTargetVariableMaps_.insert("Two way time", "TwoWayTime");
  calibrationTargetVariableMaps_.insert("TWTT", "TwoWayTime");
  calibrationTargetVariableMaps_.insert("TWT_FROM_DT", "TWT_FROM_DT");
  calibrationTargetVariableMaps_.insert("SAC-TWTT_from_DT", "TWT_FROM_DT");
  calibrationTargetVariableMaps_.insert("SAC-TWTT_from_DT_from_VP", "TWT_FROM_DT");
  calibrationTargetVariableMaps_.insert("BulkDensity", "BulkDensity");
  calibrationTargetVariableMaps_.insert("Density", "BulkDensity");
  calibrationTargetVariableMaps_.insert("SonicSlowness", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DT", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DT_From_VP", "DT_FROM_VP");
  calibrationTargetVariableMaps_.insert("DTfromVP", "DT_FROM_VP");
  calibrationTargetVariableMaps_.insert("Temperature", "Temperature");
  calibrationTargetVariableMaps_.insert("Vr", "VRe");
  calibrationTargetVariableMaps_.insert("Vre", "VRe");
  calibrationTargetVariableMaps_.insert("VRe", "VRe");
  calibrationTargetVariableMaps_.insert("GammaRay", "GammaRay");
  calibrationTargetVariableMaps_.insert("GR", "GammaRay");
  calibrationTargetVariableMaps_.insert("Pressure", "Pressure");
  calibrationTargetVariableMaps_.insert("Velocity", "Velocity");
  calibrationTargetVariableMaps_.insert("VP", "Velocity");
}

void ExtractWellDataXlsx::mapTargetVarNames()
{
  for (QString& calTargetVar : wellData_->calibrationTargetVarsUserName_)
  {
    wellData_->calibrationTargetVarsCauldronName_.push_back(calibrationTargetVariableMaps_.value(calTargetVar, "Unknown"));
  }
}

void ExtractWellDataXlsx::setWellNames() {
  for (int i_sheet = 0; i_sheet < xlsx_.sheetNames().size(); ++i_sheet)
  {
    QXlsx::Worksheet* sheet_ = dynamic_cast<QXlsx::Worksheet *>(xlsx_.sheet(xlsx_.sheetNames()[i_sheet]));
    wellNames_.append(sheet_->read("A2").toString());
  }
}

const QVector<QString>& ExtractWellDataXlsx::wellNames() const
{
  return wellNames_;
}

double ExtractWellDataXlsx::xCoord() const
{
  return wellData_->xCoord_;
}

double ExtractWellDataXlsx::yCoord() const
{
  return wellData_->yCoord_;
}

const QString& ExtractWellDataXlsx::metaData() const
{
  return wellData_->metaData_;
}

QVector<double> ExtractWellDataXlsx::depth() const
{
  return wellData_->depth_;
}

unsigned int ExtractWellDataXlsx::nCalibrationTargetVars() const
{
  return wellData_->nCalibrationTargetVars_;
}

QVector<QString> ExtractWellDataXlsx::calibrationTargetVarsUserName() const
{
  return wellData_->calibrationTargetVarsUserName_;
}

QVector<QString> ExtractWellDataXlsx::calibrationTargetVarsCauldronName() const
{
  return wellData_->calibrationTargetVarsCauldronName_;
}

QVector<unsigned int> ExtractWellDataXlsx::nDataPerTargetVar() const
{
  return wellData_->nDataPerTargetVar_;
}

QVector<double> ExtractWellDataXlsx::calibrationTargetValues() const
{
  return wellData_->calibrationTargetValues_;
}

QVector<double> ExtractWellDataXlsx::calibrationTargetStdDeviation() const
{
  return wellData_->calibrationTargetStdDeviation_;
}

QMap<QString, QString> ExtractWellDataXlsx::calibrationTargetVariableMaps() const
{
  return calibrationTargetVariableMaps_;
}

} //namespace casaWizard
