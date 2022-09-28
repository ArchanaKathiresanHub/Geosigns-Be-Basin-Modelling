//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "optimizedLithofractionScript.h"

#include "model/logger.h"
#include "model/sacScenario.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

OptimizedLithofractionScript::OptimizedLithofractionScript(const QString& baseDirectory) :
  RunScript(baseDirectory),
  scriptFilename_{"obtainLithofractions.casa"}
{
}

bool OptimizedLithofractionScript::generateCommands()
{
  const int nCalls = runFolders_.size();
  addCommand("which casa.exe");
  for (int iCall = 0; iCall < nCalls; ++iCall)
  {
    createScriptInFolder(runFolders_[iCall]);
    addCommand("casa.exe " + scriptFilename_, runFolders_[iCall]);
  }
  return true;
}

void OptimizedLithofractionScript::addCase(const QString& runFolder)
{
   runFolders_.append(runFolder);
}

bool OptimizedLithofractionScript::scriptShouldCancelWhenFailureIsEncountered() const
{
   return false;
}

void OptimizedLithofractionScript::createScriptInFolder(const QString& folderName)
{
  const QString filePath{baseDirectory() + QDir::separator() + folderName + QDir::separator() + scriptFilename_};
  QFile file{filePath};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    Logger::log() << " - Failed to write script <" << filePath << ">" << Logger::endl();
    return;
  }
  QTextStream out(&file);
  out << "loadstate \"casa_state.bin\" \"bin\"\n";
  out << "exportDataTxt \"DoEParameters\" \"optimalLithofractions.txt\" \"BestMatchedCase\"\n";
  file.close();
}

} // namespace sac

} // namespace casaWizard
