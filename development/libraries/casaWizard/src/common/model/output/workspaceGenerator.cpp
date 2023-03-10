#include "workspaceGenerator.h"

#include <QDateTime>
#include <QDir>
#include <QFile>

namespace casaWizard
{

namespace workspaceGenerator
{

QString getTimeStamp(const QString& prefix)
{
  const  QDateTime now{QDateTime::currentDateTime()};
  return prefix + now.toString("yyyyMMdd-HHmmss");
}

QString getSuggestedWorkspace(const QString &path)
{
  const QDir originWspace(path);
  const QString rootPath = originWspace.absolutePath();

  const QString workspaceName = "/casaWorkspace-" + getTimeStamp();
  return rootPath + workspaceName;
}

bool createWorkspace(const QString& currentPath, const QString& newFolderPath)
{    
  QDir newWorkspaceLocation{newFolderPath};
  newWorkspaceLocation.mkdir(newFolderPath);
  if (!newWorkspaceLocation.exists())
  {
    return false;
  }

  if (currentPath == "")
  {
     return true; // no need for copying in that case
  }

  QDir originalWorkspacelocation{currentPath};
  if (originalWorkspacelocation.exists() && currentPath == newFolderPath)
  {
    QString tempPath = newFolderPath + "_1";
    while (QDir(tempPath).exists())
    {
      tempPath += "_1";
    }
    if (copyDir(currentPath, tempPath))
    {
      originalWorkspacelocation.removeRecursively();
      originalWorkspacelocation.rename(tempPath, newFolderPath);
      return true;
    }
    return false;
  }

  return copyDir(currentPath, newFolderPath);
}

bool copyDir(const QString &src, const QString &dest)
{
  const QDir dir(src);
  QDir dirdest{dest};
  if (!dirdest.exists())
  {
    dirdest.mkdir(dest);
  }

  if (!dir.isReadable())
  {
    return false;
  }

  QFileInfoList entries = dir.entryInfoList();

  for (QFileInfo& finfo : entries)
  {
    if (!finfo.isReadable())
    {
      return false;
    }

    if (finfo.fileName().contains(".project3d") ||
        finfo.fileName().contains(".xlsx") ||
        finfo.fileName().contains(".HDF") ||
        finfo.fileName().contains(".FLT") ||
        finfo.fileName().contains(".txt"))
    {
      QFile file(finfo.filePath());
      file.copy(dirdest.absoluteFilePath(finfo.fileName()));
    }
  }
  return true;
}

} // namespace workspaceGenerator

} // namespace casaWizard
