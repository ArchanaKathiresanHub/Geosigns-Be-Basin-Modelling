//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "saveWellDataXlsx.h"

#include "model/wellData.h"

#include "xlsxworksheet.h"

namespace casaWizard
{

SaveWellDataXlsx::SaveWellDataXlsx(const QString& calibrationXlsxFileName) :
  xlsx_{calibrationXlsxFileName}
{    
}

void SaveWellDataXlsx::saveWellData(const QString& wellName, const WellData& wellData)
{
  xlsx_.addSheet(wellName);
  QXlsx::Worksheet* sheet = dynamic_cast<QXlsx::Worksheet *>(xlsx_.sheet(wellName));

  QXlsx::Format format;
  format.setFontName("Arial");
  format.setFontSize(10);
  format.setFontBold(true);
  writeSheetHeaders(sheet, format);

  format.setFontBold(false);
  sheet->write("A2", wellName, format);
  sheet->write("B2", wellData.xCoord_, format);
  sheet->write("C2", wellData.yCoord_, format);
  sheet->write("A3", wellData.metaData_, format);

  int col = 1;
  int i = 0;
  for (unsigned int var = 0; var < wellData.nCalibrationTargetVars_; ++var)
  {    
    format.setFontBold(true);
    sheet->write(4, col, wellData.calibrationTargetVarsUserName_[var], format);
    format.setFontBold(false);

    sheet->write(8, col, "Depth [m]", format);
    sheet->write(8, col + 1, wellData.calibrationTargetVarsUserName_[var], format);

    const int rowOffset = 9;
    for ( unsigned int row = 0; row < wellData.nDataPerTargetVar_[var]; ++row )
    {
      sheet->write(row + rowOffset, col, wellData.depth_[i], format);
      sheet->write(row + rowOffset, col+1, wellData.calibrationTargetValues_[i], format);
      sheet->write(row + rowOffset, col+2, wellData.calibrationTargetStdDeviation_[i], format);
      ++i;
    }
    col += 3;
  }
}

void SaveWellDataXlsx::writeSheetHeaders(QXlsx::Worksheet* sheet, const QXlsx::Format& boldFormat)
{
  sheet->write("A1", QString("Well Name"), boldFormat);
  sheet->write("B1", QString("X coordinate"), boldFormat);
  sheet->write("C1", QString("Y coordinate"), boldFormat);
}

bool SaveWellDataXlsx::save()
{
  return xlsx_.save();
}

} //namespace casaWizard
