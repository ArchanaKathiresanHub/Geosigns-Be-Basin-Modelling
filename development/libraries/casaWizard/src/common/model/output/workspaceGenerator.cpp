#include "workspaceGenerator.h"

#include <QDateTime>
#include <QDir>
#include <QFile>

namespace casaWizard
{

namespace workspaceGenerator
{

QString getSuggestedWorkspace(const QString &currentPath)
{
  const QDir originWspace = QFileInfo(currentPath).absoluteDir();
  const QString rootPath=originWspace.absolutePath();

  const  QDateTime now{QDateTime::currentDateTime()};
  const QString date{now.toString("yyyy-MM-dd-HH-mm")};

  const QString workspaceName = "/casaWorkspace-" + date;
  return rootPath + workspaceName;
}

bool createWorkspace(const QString& currentPath, const QString& newFolderPath)
{
  const QDir originalWorkspacelocation{currentPath};
  const QDir newWorkspaceLocation{newFolderPath};

  if (originalWorkspacelocation.exists() && currentPath == newFolderPath)
  {
    return true;
  }
  else
  {
    originalWorkspacelocation.mkdir(newFolderPath);
  }

  if (!newWorkspaceLocation.exists())
  {
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
        finfo.fileName().contains(".FLT"))
    {
      QFile file(finfo.filePath());
      file.copy(dirdest.absoluteFilePath(finfo.fileName()));
    }
  }
  return true;
}

} // namespace workspaceGenerator

} // namespace casaWizard
