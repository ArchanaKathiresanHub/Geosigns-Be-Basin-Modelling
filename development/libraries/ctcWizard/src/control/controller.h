#pragma once

#include "model/ctcScenario.h"
#include "view/mainWindow.h"
#include "SDUWorkLoadManager.h"
#include <QObject>
#include <QDir>

#define WAITTIME 500

class QProcess;
class QString;
class QTableWidgetItem;

namespace ctcWizard
{

class Controller : public QObject
{
  Q_OBJECT

public:
  explicit Controller();
  void executeFastcauldronScript(const QString& filePath, const QString& fastcldrnRunMode, const QString numProc="1") const;
  void executeCtcScript(const QString& filePath, const QString numProc = "1") const;
  QString createCTCscenarioFolder(const QString& filePath) const;
  void mapOutputCtcScript(const QString& filePath) const;
  void launchCauldronMapsTool(const QString& filePath);
  void log(const QString& text)const;
  void createScenarioForALC(const QString& folderPath);
  void deleteCTCscenario(const QString& folderPath);
  void showUI();

private slots:
  void slotActionExitTriggered();
  void slotActionOpenFileTriggered();
  void slotPushClearLogClicked(void) const ;

private:
  void createConnections() const;
  bool processCommand(QProcess& process, const QString& command) const;
  bool processShCommand(QProcess& process, const QString& command) const;
  bool makeDirSymLinks(const QDir& src, const QDir& desti) const;
  MainWindow ui_;
  CtcScenario ctcScenario_;
  QVector<QObject*> subControllers_;
};

} // namespace ctcWizard
