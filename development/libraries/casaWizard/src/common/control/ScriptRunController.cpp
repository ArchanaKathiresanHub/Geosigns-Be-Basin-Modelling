//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ScriptRunController.h"

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
   m_processCancelled{false},
   m_dialog{},
   m_process{new QProcess(this)},
   m_script{nullptr}
{
   m_dialog.close();
   m_dialog.setModal(false);
   QSizePolicy policy = m_dialog.sizePolicy();
   policy.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
   policy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
   m_dialog.setSizePolicy(policy);
   m_dialog.setWindowTitle("Executing");

   m_dialog.setStandardButtons(QMessageBox::StandardButton::Cancel);
   m_dialog.setDefaultButton(QMessageBox::StandardButton::Cancel);

   connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadStandardOutput()));
   connect(m_dialog.defaultButton(), SIGNAL(clicked()),  this, SLOT(killProcess()));
   connect(&m_dialog, SIGNAL(rejected()),                this, SLOT(killProcess()));
}

bool ScriptRunController::processCommand(const RunCommand& command)
{
   m_process->setWorkingDirectory(m_script->baseDirectory() + "/" + command.relativeDirectory);
   m_process->start(command.command);
   if (!m_process->waitForStarted())
   {
      m_process->kill();
      return false;
   }

   qApp->processEvents(); //When running a long series of commands that mostly complete within 100 ms, this processEvents is needed to make sure that the log gets updated.

   //If the process finishes while the code in the while loop is being executed, the wizard might get stuck in the while loop without the second statement.
   while (!m_process->waitForFinished(100) && m_process->state() == QProcess::ProcessState::Running)
   {
      if (!m_processCancelled && m_dialog.isHidden())
      {
         m_dialog.show(); //Only show waiting dialog if process takes longer than 100 ms.
      }
      qApp->processEvents();
   }

   return true;
}

bool ScriptRunController::runScript(RunScript& script, QObject * receiver, const char* slot, int timeout)
{
   m_script = &script;
   m_processCancelled = false;

   emit runStarted();

   QTimer timer;
   if (slot != nullptr)
   {
      connect(&timer, SIGNAL(timeout()), receiver, slot);
      timer.start(timeout);
   }

   if (!script.generateCommands())
   {
      emit runEnded();
      return false;
   }

   if (script.commands().empty())
   {
      emit runEnded();
      return false;
   }
   QTime time;
   time.start();
   Logger::log() << "Start processing: " << time.currentTime().toString() << Logger::endl();

   for (const RunCommand& command : script.commands())
   {
      if (m_processCancelled) break;

      m_dialog.setText( command.command );
      m_dialog.raise();
      Logger::log() << command.command << Logger::endl();

      if (!processCommand(command))
      {
         Logger::log() << "Last command failed: " << command.command << Logger::endl();
         m_processCancelled = true;
         break;
      }
   }
   if (m_processCancelled)
   {
      Logger::log() << "Command pipeline interrupted" << Logger::endl();
      m_dialog.close();
   }
   else
   {
      m_dialog.accept();
   }

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

   emit runEnded();

   m_script = nullptr;
   return !m_processCancelled;
}

QByteArray ScriptRunController::processOutput()
{
   QByteArray outcpy = m_processOutputs;
   m_processOutputs.clear();
   return outcpy;
}

void ScriptRunController::killProcess()
{
   m_processCancelled = true;
   Logger::log() << "Preparing interruption ... "  << Logger::endl();
   if (m_script != nullptr && m_script->killAsync())
   {
      Logger::log() << "Interrupting running process" << Logger::endl();
   }
   else
   {
      m_process->terminate();
      Logger::log() << "Killed running process" << Logger::endl();
   }
}

void ScriptRunController::slotReadStandardOutput()
{
   m_processOutputs.append(m_process->readAllStandardOutput());
   QString outStr(m_processOutputs);

   if (outStr.contains("Basin_Fatal") && m_script->scriptShouldCancelWhenFailureIsEncountered())
   {
      m_processCancelled = true;
   }

   emit readyReadStandardOutput();
}

} // namespace casaWizard
