// Class to execute casa scripts
#pragma once

#include <QByteArray>
#include <QObject>
#include <QProgressDialog>
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
  ScriptRunController(QObject* parent = 0);

  bool runScript(RunScript& script);
  QByteArray readAllStandardOutput();

signals:
  void readyReadStandardOutput();

private slots:
  void killProcess();

private:
  bool processCommand(const RunCommand& command);

  bool processCancelled_;
  QProgressDialog dialog_;
  QProcess* process_;
  QString baseDirectory_;
  RunScript* script_;
};

} // namespace casaWizard
