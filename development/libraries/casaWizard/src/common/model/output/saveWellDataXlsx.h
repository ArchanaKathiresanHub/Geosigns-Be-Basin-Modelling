//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Writer for an Excel data file with calibration targets order by well and variable type
#pragma once

#include "xlsxdocument.h"

#include <QString>
#include <QVector>

#include <memory>

namespace casaWizard
{

struct WellData;

class SaveWellDataXlsx
{
public:
  SaveWellDataXlsx(const QString& xlsxFileName = "");  

  void saveWellData(const QString& wellName, const WellData& wellData);
  bool save();

private:  

  QXlsx::Document xlsx_;  

  void writeSheetHeaders(QXlsx::Worksheet* sheet, const QXlsx::Format& boldFormat);
};

} // namespace casaWizard
