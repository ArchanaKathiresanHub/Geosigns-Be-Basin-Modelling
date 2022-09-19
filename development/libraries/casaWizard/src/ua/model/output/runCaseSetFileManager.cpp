//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "runCaseSetFileManager.h"

#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include <QDirIterator>
#include <QVector>

#include <algorithm>
#include <iostream>

namespace casaWizard
{

namespace ua
{

namespace
{

double convertByteToMegaByte(const double sizeInBytes)
{
   return sizeInBytes/1024.0/1024.0;
}

} // namespace

RunCaseSetFileManager::RunCaseSetFileManager():
   iterationDirFileInfo_{QFileInfo{""}}
{
}

QFileInfoList RunCaseSetFileManager::getIterationPathList(const QString& runLocationPath) const
{
   QDir caseSetPath{runLocationPath};
   return caseSetPath.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Time | QDir::Reversed);
}

QString RunCaseSetFileManager::iterationDirPath() const
{
   return iterationDirFileInfo_.absoluteFilePath();
}

QString RunCaseSetFileManager::caseSetDirPath() const
{
   return iterationDirFileInfo_.absoluteDir().absolutePath();
}

void RunCaseSetFileManager::setIterationPath(const QString& runLocationPath)
{
   clear();
   QFileInfoList iterationPathList = getIterationPathList(runLocationPath);

   if (iterationPathList.isEmpty())
   {
      return;
   }

   QFileInfo lastIterationDir = iterationPathList.last();
   while (!lastIterationDir.fileName().contains("Iteration", Qt::CaseSensitive) && !iterationPathList.isEmpty())
   {
      iterationPathList.pop_back();

      if (iterationPathList.isEmpty())
      {
         return;
      }

      lastIterationDir = iterationPathList.last();
   }

   iterationDirFileInfo_ = lastIterationDir;
}

double RunCaseSetFileManager::iterationDirFilesSize() const
{
   QDirIterator itDir(iterationDirPath(), QDir::Files, QDirIterator::Subdirectories);
   qint64 dirSize = 0;
   while (itDir.hasNext())
   {
      QFile f(itDir.next());
      dirSize += f.size();
   }

   return convertByteToMegaByte(dirSize);
}

double RunCaseSetFileManager::allIterationDirsFilesSize(const QString& project3dPath) const
{
   QFileInfo project3dPathFileInfo{project3dPath};
   QDirIterator itDir(project3dPathFileInfo.absolutePath() + QString("/CaseSet"), QDir::Files, QDirIterator::Subdirectories);
   qint64 dirSize = 0;
   while (itDir.hasNext())
   {
      QFile f(itDir.next());
      dirSize += f.size();
   }

   return convertByteToMegaByte(dirSize);
}

bool RunCaseSetFileManager::removeIterationDir()
{
   if (isIterationDirEmpty())
   {
      return false;
   }

   return QDir(iterationDirPath()).removeRecursively();
}

bool RunCaseSetFileManager::removeAllIterationDirs(const QString& runLocationPath)
{
   QFileInfo project3dPathFileInfo{runLocationPath};
   QDir caseSetDir{project3dPathFileInfo.absolutePath() + QString("/CaseSet")};

   return caseSetDir.removeRecursively();
}

bool RunCaseSetFileManager::isIterationDirDeleted(const QString& runLocationPath) const
{
   QFileInfoList iterationPathList = getIterationPathList(runLocationPath);
   return !iterationPathList.contains(iterationDirPath());
}

bool RunCaseSetFileManager::isIterationDirEmpty() const
{
   return iterationDirPath().isEmpty();
}

QString RunCaseSetFileManager::iterationDirName() const
{
   return iterationDirFileInfo_.fileName();
}

void RunCaseSetFileManager::writeToFile(ScenarioWriter& writer) const
{
   writer.writeValue("RunCaseSetFileManagerVersion", 1);
   writer.writeValue("iterationDirFileInfo",  + "./CaseSet/" + iterationDirName());
}

void RunCaseSetFileManager::readFromFile(const ScenarioReader& reader)
{
   const QString& iterationDirFileInfoStr = reader.readString("iterationDirFileInfo");
   const QString& wDir = reader.readString("workingDirectory");
   iterationDirFileInfo_ = QFileInfo{wDir + "/" + iterationDirFileInfoStr};
}

void RunCaseSetFileManager::clear()
{
   iterationDirFileInfo_ = {};
}

} // namespace ua

} // namespace casaWizard
