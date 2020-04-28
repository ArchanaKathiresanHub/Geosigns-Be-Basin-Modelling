#include "scriptRunController.h"

#include "model/logger.h"
#include "model/script/runScript.h"

#include <QApplication>
#include <QDialog>
#include <QProcess>

namespace casaWizard
{

ScriptRunController::ScriptRunController(QObject* parent) :
  QObject(parent),
  dialog_{},
  process_{new QProcess(this)},
  baseDirectory_{""},
  script_{}
{
  dialog_.close();
  dialog_.setWindowModality(Qt::ApplicationModal);
  dialog_.setFixedWidth(400);
  dialog_.setWindowTitle("Executing");

  connect(process_, SIGNAL(readyReadStandardOutput()), this, SIGNAL(readyReadStandardOutput()));
  connect(&dialog_, SIGNAL(canceled()),                this, SLOT(killProcess()));
}

bool ScriptRunController::processCommand(const RunCommand& command)
{
  process_->setWorkingDirectory(script_->baseDirectory() + "/" + command.relativeDirectory);
  process_->start(command.command);
  if (!process_->waitForStarted())
  {
    process_->kill();
    return false;
  }

  while (!process_->waitForFinished(100))
  {
    qApp->processEvents();
  }

  return true;
}

bool ScriptRunController::runScript(RunScript& script)
{
  script_ = &script;

  if (!script.generateCommands())
  {
    return false;
  }

  if (script.commands().empty())
  {
    return false;
  }

  dialog_.reset();
  dialog_.setRange(0,script.commands().size());
  dialog_.show();

  int i{0};
  for (const RunCommand& command : script.commands())
  {
    if (dialog_.wasCanceled())
    {
      return false;
    }

    dialog_.setLabelText("Running: " + command.command);
    Logger::log() << command.command << Logger::endl();

    if (!processCommand(command))
    {
      Logger::log() << "Command pipeline interrupted, because last command failed" << Logger::endl();
      dialog_.setValue(dialog_.maximum());
      return false;
    }
    dialog_.setValue(++i);
  }

  if (dialog_.wasCanceled())
  {
    return false;
  }

  return true;
}

QByteArray ScriptRunController::readAllStandardOutput()
{
  return process_->readAllStandardOutput();
}

void ScriptRunController::killProcess()
{
  Logger::log() << "Preparing interruption ... "  << Logger::endl();
  if (script_->prepareKill())
  {
    process_->terminate();
    Logger::log() << "Interrupting running process" << Logger::endl();
  }
}

} // namespace casaWizard
