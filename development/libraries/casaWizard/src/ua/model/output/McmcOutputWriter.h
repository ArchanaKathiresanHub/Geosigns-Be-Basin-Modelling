//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/writable.h"

#include <QFileInfo>
#include <QString>
#include <QVector>

namespace casaWizard
{

namespace ua
{

struct McmcTargetExportData;
struct McmcSingleTargetExportData;

class McmcOutputWriter
{
public:
   static bool writeToFile(QString fileName, const McmcTargetExportData& exportData);

private:
   McmcOutputWriter(QString fileName, const McmcTargetExportData& exportData);

   bool writeTarget(const McmcSingleTargetExportData& d);

   bool writeToFilePrivate();
   bool writeHeader();
   bool writeData();

   QString m_fileName;
   const McmcTargetExportData& m_exportData;
   QByteArray m_writeData;
};

} // namespace ua

} // namespace casaWizard
