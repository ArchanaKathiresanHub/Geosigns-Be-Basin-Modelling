#include "scriptRunController.h"

#include "model/logger.h"
#include "model/script/runScript.h"

#include <QApplication>
#include <QDialog>
#include <QProcess>
#include <QPushButton>
#include <QTime>
#include <QTimer>

namespace casaWizard
{

ScriptRunController::ScriptRunController(QObject* parent) :
  QObject(parent),
  processCancelled_{false},
  dialog_{},
  process_{new QProcess(this)},
  baseDirectory_{""},
  script_{}
{
  dialog_.close();
  dialog_.setModal(false);
  QSizePolicy policy = dialog_.sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
  policy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
  dialog_.setSizePolicy(policy);
  dialog_.setWindowTitle("Executing");

  dialog_.setStandardButtons(QMessageBox::StandardButton::Cancel);
  dialog_.setDefaultButton(QMessageBox::StandardButton::Cancel);

  connect(process_, SIGNAL(readyReadStandardOutput()), this, SIGNAL(readyReadStandardOutput()));
  connect(dialog_.defaultButton(), SIGNAL(clicked()),  this, SLOT(killProcess()));
  connect(&dialog_, SIGNAL(rejected()),                this, SLOT(killProcess()));
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

bool ScriptRunController::runScript(RunScript& script, QObject * receiver, const char* slot, int timeout)
{
  QTimer timer;
  if (slot != nullptr)
  {
    connect(&timer, SIGNAL(timeout()), receiver, slot);
    timer.start(timeout);
  }

  processCancelled_ = false;
  script_ = &script;

  if (!script.generateCommands())
  {
    return false;
  }

  if (script.commands().empty())
  {
    return false;
  }
  QTime time;
  time.start();
  Logger::log() << "Start processing: " << time.currentTime().toString() << Logger::endl();

  dialog_.show();

  for (const RunCommand& command : script.commands())
  {
    if (processCancelled_) break;

    dialog_.setText( command.command );
    dialog_.raise();
    Logger::log() << command.command << Logger::endl();

    if (!processCommand(command))
    {
      Logger::log() << "Last command failed: " << command.command << Logger::endl();
      processCancelled_ = true;
      break;
    }
  }
  if (processCancelled_)
  {
    dialog_.reject();
    Logger::log() << "Command pipeline interrupted" << Logger::endl();
  }
  else
  {
    dialog_.accept();
    if (slot != nullptr)
    {
      timer.stop();
      QMetaObject::invokeMethod(receiver, slot);
    }

    int elapsed = time.elapsed();
    const int hours = elapsed/3600000; elapsed = elapsed % 3600000;
    const int minutes = elapsed/60000; elapsed = elapsed % 60000;
    const int seconds = elapsed/1000; elapsed = elapsed % 1000;
    Logger::log() << "Elapsed time: " << hours <<"h:" << minutes <<"m:" << seconds <<"s:" << elapsed <<"ms" << Logger::endl();
    Logger::log() << "Finished processing: " << time.currentTime().toString() << Logger::endl();
  }

  return !processCancelled_;
}

QByteArray ScriptRunController::readAllStandardOutput()
{
  return process_->readAllStandardOutput();
}

void ScriptRunController::killProcess()
{
  processCancelled_ = true;
  Logger::log() << "Preparing interruption ... "  << Logger::endl();
  if (script_->prepareKill())
  {
    process_->terminate();
    Logger::log() << "Interrupting running process" << Logger::endl();
  }
}

} // namespace casaWizard
