//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "OptimalCaseExporter.h"

#include "model/functions/folderOperations.h"
#include "model/logger.h"

#include <QDir>
#include <QFile>

namespace casaWizard
{

namespace ua
{

namespace optimalCaseExporter
{

bool tryCopy(const QString& fromFolder, const QString& toFolder, const QString& fileName)
{
   QFile sourceFile(fromFolder + QDir::separator() + fileName);
   QString targetPath(toFolder + QDir::separator() + fileName);
   QFile targetFile(toFolder + QDir::separator() + fileName);

   if (targetFile.exists())
   {
      targetFile.remove();
   }

   if (!sourceFile.copy(targetPath))
   {
      Logger::log() << fileName + " does not exist. Please add it to working directory: " << fromFolder << Logger::endl();
      return false;
   }
   return true;
}

bool checkExists(const QString& path)
{
   if (!QFile::exists(path))
   {
      Logger::log() << path + " does not exist. Unable to export the optimal case." << Logger::endl();
      return false;
   }
   return true;
}

void exportOptimalCase(const QString& optimalCaseDirectory, const QString& workingDirectory)
{
   if (!checkExists(optimalCaseDirectory + "/Project.project3d")) return;

   if (!tryCopy(workingDirectory,optimalCaseDirectory,"Project.txt")) return;
   if (!tryCopy(workingDirectory,optimalCaseDirectory,"Inputs.HDF")) return;

   QDir tmpdir(optimalCaseDirectory + "/tmpDirectory");
   tmpdir.removeRecursively();
   tmpdir.mkdir(optimalCaseDirectory + "/tmpDirectory");

   if (!functions::copyCaseFolder(QDir(optimalCaseDirectory), tmpdir))
   {
      Logger::log() << "Failed exporting, no files were copied" << Logger::endl();
      tmpdir.removeRecursively();
      return;
   }

   if (!tryCopy(optimalCaseDirectory,tmpdir.absolutePath(),"Project.txt"))
   {
      tmpdir.removeRecursively();
      return;
   }

   functions::zipFolderContent(tmpdir, optimalCaseDirectory, "optimal");
   tmpdir.removeRecursively();
}

} //optimalCaseExporter
} //ua
} //casaWizard
