//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "casaScriptWriter.h"

#include "model/casaScenario.h"
#include "model/logger.h"
#include "model/script/casaScript.h"
#include "view/textEditDialog.h"

#include <QFile>
#include <QMessageBox>
#include <QTextStream>

namespace casaWizard
{

namespace casaScriptWriter
{

void modifyCasaScript(const QString& filename)
{
  Logger::log() << "- Expert modification of casa script started" << Logger::endl();
  QFile file(filename);
  if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
  {
    Logger::log() << "- Failed to modify casa script, as file can not be opened" << Logger::endl();
  }

  QTextStream textStream(&file);
  TextEditDialog dialog(textStream.readAll());
  dialog.setWindowTitle("Expert modification of CASA script " + filename);
  dialog.setMinimumSize(800,500);

  if (dialog.exec() == QDialog::Accepted)
  {
    file.reset();
    file.resize(0);
    textStream << dialog.plainText();
    Logger::log() << "- Expert modification of CASA script successful" << Logger::endl();
  }
  else
  {
    Logger::log() << "- Expert modification of CASA script cancelled" << Logger::endl();
  }

  file.close();
}

bool writeCasaScript(const CasaScript& script)
{
  if (!script.writeScript())
  {
    Logger::log() << "- Failed to write CASA script " << script.scriptFilename() << Logger::endl();
    return false;
  }

  if (script.scenario().expertUser())
  {
    modifyCasaScript(script.scenario().workingDirectory() + "/" + script.scriptFilename());
  }

  return true;
}

} // namespace casaScriptWriter

} // namespace casaWizard
