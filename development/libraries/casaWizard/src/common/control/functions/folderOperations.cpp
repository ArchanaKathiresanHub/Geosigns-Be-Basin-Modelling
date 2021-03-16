#include "folderOperations.h"

#include "model/logger.h"

#include <QMessageBox>

namespace casaWizard
{

namespace functions
{

bool copyCaseFolder(const QDir sourceDir, const QDir targetDir)
{
  bool filesCopied = false;
  if (!targetDir.exists())
  {
    QMessageBox doesNotExist(QMessageBox::Icon::Question,
                             "Destination folder not found",
                             "The destination folder does not exist, do you want to create it and proceed?",
                             QMessageBox::Yes | QMessageBox::No);
    if (doesNotExist.exec() == QMessageBox::Yes)
    {
      targetDir.mkpath(".");
      Logger::log() << "Created the folder " + targetDir.absolutePath() << Logger::endl();
    }
    else
    {
      return false;
    }
  }

  QStringList targetDuplicates;
  QStringList sourceDuplicates;
  for (const QFileInfo& file : sourceDir.entryInfoList(QDir::Files))
  {
    const QString suffix = file.completeSuffix().toLower();
    if (suffix == "project3d" || suffix == "hdf" || suffix == "flt")
    {
      const QString sourcePath = file.absoluteFilePath();
      const QString targetPath = targetDir.absolutePath() + QDir::separator() + file.fileName();
      if (QFile::exists(targetPath))
      {
        targetDuplicates << targetPath;
        sourceDuplicates << sourcePath;
      }
      else
      {
        QFile::copy(sourcePath, targetPath);
        Logger::log() << "Copied \"" << sourcePath << "\" to \"" << targetPath << "\"" << Logger::endl();
        filesCopied = true;
      }
    }
  }

  const int nrDuplicates = targetDuplicates.size();
  if (nrDuplicates == 0)
  {
    return filesCopied;
  }

  QMessageBox messageBox;
  messageBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
  if (nrDuplicates == 1)
  {
    messageBox.setText("The file \"" + targetDuplicates[0] + "\" already exists.");
    messageBox.setInformativeText("Do you want to overwrite it?");
  }
  else
  {
    messageBox.setText("The files: \n\"" + targetDuplicates.join("\"\n\"") + "\"\n already exist");
    messageBox.setInformativeText("Do you want to overwrite them?");
  }

  if (messageBox.exec() == QMessageBox::Yes)
  {
    for (int i = 0; i < nrDuplicates; ++i)
    {
      const QString& sourcePath = sourceDuplicates[i];
      const QString& targetPath = targetDuplicates[i];
      QFile::remove(targetPath);
      QFile::copy(sourcePath, targetPath);
      Logger::log() << "Copied \"" << sourcePath << "\" to \"" << targetPath << "\"" << Logger::endl();
    }
    filesCopied = true;
  }

  return filesCopied;
}

bool removeIfUserAgrees(const QString& directory)
{
  if (QDir(directory).exists())
  {
    QMessageBox doesExist(QMessageBox::Icon::Question,
                          "Directory exists.",
                          "Delete directory: " + directory + " ?",
                          QMessageBox::Yes | QMessageBox::No);
    if (doesExist.exec() != QMessageBox::Yes)
    {
      return false;
    }    
  }
  return true;
}

} // namespace functions

} // namespace casaWizard
