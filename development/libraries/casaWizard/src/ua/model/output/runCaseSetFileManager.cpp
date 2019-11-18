#include "runCaseSetFileManager.h"

#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include <QDirIterator>
#include <QVector>

#include <algorithm>

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
  iterationDirFileInfo_{QFileInfo{""}},
  iterationDirDateTime_{}
{
}

QFileInfoList RunCaseSetFileManager::getIterationPathList(const QString& project3dPath) const
{
  QDir caseSetPath{QString("./CaseSet")};
  return caseSetPath.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Time | QDir::Reversed);
}

QDateTime RunCaseSetFileManager::iterationDirDateTime() const
{
  return iterationDirDateTime_;
}

QString RunCaseSetFileManager::iterationDirPath() const
{
  return iterationDirFileInfo_.absoluteFilePath();
}

QString RunCaseSetFileManager::caseSetDirPath() const
{
  return iterationDirFileInfo_.absoluteDir().absolutePath();
}

void RunCaseSetFileManager::setIterationPath(const QString& project3dPath)
{
  clear();
  QFileInfoList iterationPathList = getIterationPathList(project3dPath);

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
      lastIterationDir = QFileInfo{""};
    }
    else
    {
      lastIterationDir = iterationPathList.last();
    }
  }

  if (lastIterationDir.fileName().isEmpty())
  {
    return;
  }

  iterationDirFileInfo_ = lastIterationDir;
  iterationDirDateTime_ = lastIterationDir.created();
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

bool RunCaseSetFileManager::removeAllIterationDirs(const QString& project3dPath)
{
  QFileInfo project3dPathFileInfo{project3dPath};
  QDir caseSetDir{project3dPathFileInfo.absolutePath() + QString("/CaseSet")};

  return caseSetDir.removeRecursively();
}

bool RunCaseSetFileManager::isIterationDirDeleted(const QString& project3dPath) const
{
  QFileInfoList iterationPathList = getIterationPathList(project3dPath);

  if (iterationPathList.isEmpty())
  {
    return true;
  }

  QFileInfo myIterationDir = iterationPathList.last();
  while (!(myIterationDir.absoluteFilePath() == iterationDirPath()
         && myIterationDir.lastModified() == iterationDirDateTime_)
         && !iterationPathList.isEmpty())
  {
    iterationPathList.pop_back();

    if (iterationPathList.isEmpty())
    {
      myIterationDir = QFileInfo{""};
    }
    else
    {
      myIterationDir = iterationPathList.last();
    }
  }

  return myIterationDir.fileName().isEmpty();
}

bool RunCaseSetFileManager::isIterationDirEmpty() const
{
  return iterationDirPath().isEmpty();
}

QString RunCaseSetFileManager::iterationDirName() const
{
  return iterationDirFileInfo_.fileName();
}

void RunCaseSetFileManager::updateIterationDirFileDateTime()
{
  iterationDirFileInfo_ = QFileInfo(iterationDirFileInfo_.absoluteFilePath());
  iterationDirDateTime_ = iterationDirFileInfo_.lastModified();
}

void RunCaseSetFileManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("RunCaseSetFileManagerVersion", 0);
  writer.writeValue("iterationDirFileInfo",  + "./CaseSet/" + iterationDirName());
  writer.writeValue("iterationDirDateTime", iterationDirDateTime_.toString());
}

void RunCaseSetFileManager::readFromFile(const ScenarioReader& reader)
{
  const QString& iterationDirFileInfoStr = reader.readString("iterationDirFileInfo");
  iterationDirFileInfo_ = QFileInfo{iterationDirFileInfoStr};
  const QString& iterationDirDateTimeStr = reader.readString("iterationDirDateTime");
  iterationDirDateTime_ = QDateTime::fromString(iterationDirDateTimeStr);
}

void RunCaseSetFileManager::clear()
{
  iterationDirFileInfo_ = {};
  iterationDirDateTime_ = {};
}

} // namespace ua

} // namespace casaWizard
