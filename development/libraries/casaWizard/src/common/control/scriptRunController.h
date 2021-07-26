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
  QByteArray readAllStandardOutput();

signals:
  void readyReadStandardOutput();

private slots:
  void killProcess();

private:
  bool processCommand(const RunCommand& command);

  bool processCancelled_;
  QMessageBox dialog_;
  QProcess* process_;
  QString baseDirectory_;
  RunScript* script_;
};

} // namespace casaWizard
