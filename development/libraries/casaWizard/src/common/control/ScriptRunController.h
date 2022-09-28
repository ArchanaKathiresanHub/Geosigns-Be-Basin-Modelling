//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Class to execute casa scripts
#pragma once

#include <QByteArray>
#include <QObject>
#include <QMessageBox>
#include <QString>

class QProcess;

namespace casaWizard
{

class RunScript;
struct RunCommand;

class ScriptRunController : public QObject
{
  Q_OBJECT

public:
  ScriptRunController(QObject* parent);

  bool runScript(RunScript& script, QObject* receiver = nullptr, const char* slot = nullptr, int timeout = 5000);
  QByteArray processOutput();

signals:
  void readyReadStandardOutput();
  void runStarted();
  void runEnded();

private slots:
  void killProcess();
  void slotReadStandardOutput();

private:
  bool processCommand(const RunCommand& command);

  bool m_processCancelled;
  QMessageBox m_dialog;
  QProcess* m_process;
  RunScript* m_script;
  QByteArray m_processOutputs;
};

} // namespace casaWizard
