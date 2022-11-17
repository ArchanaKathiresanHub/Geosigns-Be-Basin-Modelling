#include "casaScript.h"

#include "model/casaScenario.h"
#include "model/logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

namespace casaWizard
{

CasaScript::CasaScript(const QString& baseDirectory) :
   RunScript(baseDirectory)
{
}

QString CasaScript::relativeDirectory() const
{
   return "";
}

bool CasaScript::generateCommands()
{
   Logger::log() << "- Running CASA scenario file" << Logger::endl();

   removeStopExecFile();
   addCommand("which casa.exe");
   addCommand("casa.exe " + workingDirectory() + "/" + scriptFilename(), relativeDirectory());

   return true;
}

bool CasaScript::killAsync() const
{  
   return createStopExecFile();
}

bool CasaScript::writeScript() const
{
   if (!validateBaseScenario())
   {
      return false;
   }

   if (!validateScenario())
   {
      return false;
   }

   QString filename = workingDirectory() + "/" + scriptFilename();
   QFile file{filename};

   QFileInfo info(file);
   QDir dir(info.absoluteDir());
   if (!dir.exists())
   {
      dir.mkpath(dir.absolutePath());
   }

   if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      Logger::log() << "- Failed to write casa script, as file can not be opened" << Logger::endl();
      return false;
   }

   Logger::log() << "- Start writing casa scenario file to " << filename << Logger::endl();

   try
   {
      writeScriptContents(file);
   }
   catch (const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
      return false;
   }

   file.close();
   Logger::log() << "- Finished writing casa scenario file to " << filename << Logger::endl();
   return true;
}

bool CasaScript::createStopExecFile() const
{  
   QFile stopExecFile{baseDirectory() + QDir::separator() + stopExecFilename_};
   const bool success = stopExecFile.open(QFile::OpenModeFlag::WriteOnly);
   stopExecFile.close();

   if (success)
   {
      Logger::log() << "Created " << stopExecFilename_ << " in folder " << baseDirectory() << Logger::endl();
   }
   else
   {
      Logger::log() << "Failed to create " << stopExecFilename_ << " in folder " << baseDirectory()
                    << "\n RunManager will not clean up running jobs" << Logger::endl();
   }

   return success;
}

QString CasaScript::writeDoeTextList(const QStringList& doeList) const
{
   return QString("\"" + doeList.join(",") + "\"");
}

bool CasaScript::validateBaseScenario() const
{
   if (scenario().project3dPath().isEmpty())
   {
      Logger::log() << "- Failed to write casa script, please provide the path to the project 3D file" << Logger::endl();
      return false;
   }
   return true;
}

void CasaScript::removeStopExecFile() const
{
   QFile stopExecFile{baseDirectory() + QDir::separator() + stopExecFilename_};
   if (stopExecFile.exists())
   {
      stopExecFile.remove();
   }
}

QString CasaScript::writeApp(int numberOfCPUs, const QString& applicationName) const
{
   return QString("app " + QString::number(numberOfCPUs) + " " + applicationName + "\n");
}

QString CasaScript::writeBaseProject(const QString& project3dPath) const
{
   return QString("base_project \"" + project3dPath + "\"\n");
}

QString CasaScript::writeLocation(const QString& location, const bool append, const bool noOptimization, const bool removeModelFromMemory) const
{
   return QString("location \"" + location + "\"" + ((append)?" \"append\"":"") +
                  ((noOptimization)?" \"noOptimization\"":"") +
                  ((removeModelFromMemory)?" \"removeModel\"":"") + "\n");
}

QString CasaScript::writeRun(const QString& clusterName) const
{
   return QString("run \"" + clusterName + "\" \"Default\"\n");
}

QString CasaScript::writeRunDataDigger() const
{
   return QString("runDataDigger\n");
}

QString CasaScript::writeSaveState(const QString& stateFilename) const
{
   return QString("savestate \"" + stateFilename + "\" \"txt\"\n");
}

QString CasaScript::writeLoadState(const QString& stateFilename) const
{
   return QString("loadstate \"" + stateFilename + "\" \"txt\"\n");
}

} // namespace casaWizard
