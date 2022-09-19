//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Manager class for handling run caseSet directory file features
#pragma once

#include "model/writable.h"

#include <QFileInfo>

#include <QString>

namespace casaWizard
{

namespace ua
{

class RunCaseSetFileManager : public Writable
{
public:
   explicit RunCaseSetFileManager();
   virtual ~RunCaseSetFileManager() = default;

   void setIterationPath(const QString& runLocationPath);
   double iterationDirFilesSize() const;
   double allIterationDirsFilesSize(const QString& runLocationPath) const;
   bool isIterationDirEmpty() const;
   bool removeIterationDir();
   bool removeAllIterationDirs(const QString& runLocationPath);
   QString iterationDirName() const;
   bool isIterationDirDeleted(const QString& runLocationPath) const;
   QString iterationDirPath() const;
   QString caseSetDirPath() const;

   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;

private:
   QFileInfoList getIterationPathList(const QString& runLocation) const;

   QFileInfo iterationDirFileInfo_;
};

} // namespace ua

} // namespace casaWizard
