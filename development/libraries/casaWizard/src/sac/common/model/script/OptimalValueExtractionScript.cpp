#include "OptimalValueExtractionScript.h"

#include "model/logger.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

OptimalValueExtractionScript::OptimalValueExtractionScript(const QString& baseDirectory, QString scriptFileName) :
   RunScript(baseDirectory),
   scriptFilename_{scriptFileName}
{}

bool OptimalValueExtractionScript::generateCommands()
{
   const int nCalls = runFolders_.size();
   addCommand("which", QStringList() << "casa.exe");
   for (int iCall = 0; iCall < nCalls; ++iCall)
   {
      createScriptInFolder(runFolders_[iCall]);
      addCommand("casa.exe", QStringList() << scriptFilename_, runFolders_[iCall]);
   }
   return true;
}

void OptimalValueExtractionScript::addCase(const QString& runFolder)
{
   runFolders_.append(runFolder);
}

bool OptimalValueExtractionScript::scriptShouldCancelWhenFailureIsEncountered() const
{
   return false;
}

void OptimalValueExtractionScript::createScriptInFolder(const QString& folderName)
{
   const QString filePath{baseDirectory() + QDir::separator() + folderName + QDir::separator() + scriptFilename_};
   QFile file{filePath};
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      Logger::log() << " - Failed to write script <" << filePath << ">" << Logger::endl();
      return;
   }
   writeScript(&file);
   file.close();
}

} // namespace sac

} // namespace casaWizard
