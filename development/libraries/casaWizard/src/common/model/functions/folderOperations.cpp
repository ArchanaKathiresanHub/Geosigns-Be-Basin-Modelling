#include "folderOperations.h"

#include "model/logger.h"
#include "model/output/cmbProjectWriter.h"
#include "model/output/infoGenerator.h"
#include "model/output/projectTXTManipulator.h"
#include "model/output/workspaceGenerator.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

namespace casaWizard
{

namespace functions
{

bool copyCaseFolder(const QDir sourceDir, const QDir targetDir)
{
   bool filesCopied = false;
   if (!targetDir.exists())
   {
      if (QMessageBox::question(nullptr, "Destination folder not found",
                                "The destination folder does not exist, do you want to create it and proceed?",
                                QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
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

void zipFolderContent(const QDir& sourceDir, const QString& targetDestination, const QString& zipName)
{
   QProcess process;
   process.setWorkingDirectory(targetDestination);
   processCommand(process, QString("zip -r -j " + zipName + ".zip " + sourceDir.absolutePath()));
}

void processCommand(QProcess& process, const QString& command)
{
   process.start(command);
   if (!process.waitForStarted())
   {
      process.kill();
   }

   while (!process.waitForFinished())
   {
      qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
   }
}

bool overwriteIfDirectoryExists(const QString& directory)
{
   if (QDir(directory).exists())
   {
      QMessageBox doesExist(QMessageBox::Icon::Question,
                            "Directory exists.",
                            "This will overwrite the existing directory " + directory + ".\n" +
                                "If you wish to keep that directory, you must rename it first before proceeding.\n" +
                                "Do you want to continue anyway?",
                            QMessageBox::Yes | QMessageBox::No);
      if (doesExist.exec() != QMessageBox::Yes)
      {
         return false;
      }
   }
   return true;
}

QString exportScenarioToZip(const QDir& sourceDir, const QString& workingDirectory, const QString& projectFile, InfoGenerator& infoGenerator)
{
   if (!sourceDir.exists())
   {
      Logger::log() << "Source directory for the export is not available" << Logger::endl();
      return "";
   }

   QString targetPath = QFileDialog::getExistingDirectory(nullptr, "Choose a location for the exported scenario", workingDirectory,
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

   QDir targetDir(targetPath);
   if (!targetDir.exists() || targetPath == "")
   {
      Logger::log() << "Target directory is not set" << Logger::endl();
      return "";
   }

   const QString timeStamp = workspaceGenerator::getTimeStamp("SAC_");

   QDir tmpdir(workingDirectory + "/tmpDirectory");
   tmpdir.removeRecursively();
   tmpdir.mkdir(workingDirectory + "/tmpDirectory");

   if (!functions::copyCaseFolder(sourceDir, tmpdir))
   {
      Logger::log() << "Failed exporting, no files were copied" << Logger::endl();
      tmpdir.removeRecursively();
      return "";
   }

   QString projectTextFile = projectFile;
   projectTextFile.replace(QString("project3d"), QString("txt"));
   if (!QFile::copy(workingDirectory + "/" + projectTextFile, tmpdir.absolutePath() + "/" + projectTextFile))
   {
      Logger::log() << "Project.txt does not exist, please add it to working directory: " << workingDirectory << Logger::endl();
      tmpdir.removeRecursively();
      return "";
   }

   infoGenerator.setFileName((targetDir.absolutePath() + "/" + timeStamp + "_info.txt").toStdString());
   infoGenerator.loadProjectReader(tmpdir.absolutePath().toStdString());
   infoGenerator.generateInfoTextFile();

   CMBProjectWriter writer(tmpdir.absolutePath() + "/" + projectFile);
   writer.generateOutputProject(timeStamp, workingDirectory + "/" + projectFile);
   ProjectTXTManipulator manipulator(tmpdir.absolutePath() + "/" + projectTextFile);

   try
   {
      manipulator.appendStampToScenarioName(timeStamp);
   }
   catch (const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
      tmpdir.removeRecursively();
      return "";
   }


   cleanFolder(tmpdir);

   zipFolderContent(tmpdir, targetDir.absolutePath(), timeStamp);

   tmpdir.removeRecursively();

   Logger::log() << "Finished exporting the scenario to " + targetDir.absolutePath() + "/*.zip" << Logger::endl();
   return targetDir.absolutePath();
}

void cleanFolder(const QDir& folder)
{
   for (const QFileInfo& file : folder.entryInfoList(QDir::Files))
   {
      const QString suffix = file.completeSuffix().toLower();
      if (suffix == "hdf" && file.fileName() != "Inputs.HDF")
      {
         const QString absPath = file.absoluteFilePath();
         QFile(absPath).remove();
      }
   }
}

} // namespace functions

} // namespace casaWizard
