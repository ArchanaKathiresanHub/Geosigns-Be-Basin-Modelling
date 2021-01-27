#pragma once

#include "model/ctcScenario.h"
#include "view/mainWindow.h"

#include <QObject>

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
  void executeFastcauldronScript(const QString& filePath, const QString& fastcldrnRunMode, const QString& numProc) const;
  void executeCtcScript(const QString& filePath) const;
  QString createCTCscenarioFolder(const QString& filePath) const;
  void mapOutputCtcScript(const QString& filePath) const;
  void launchCauldronMapsTool(const QString& filePath);
  void log(const QString& text)const;
  void createScenarioForALC(const QString& folderPath);
  void deleteAllCTCscenarios(const QString& folderPath);

private slots:
  void slotActionExitTriggered();
  void slotActionOpenFileTriggered();
  void slotPushClearLogClicked();

private:
  void createConnections() const;
  void processCommand(QProcess& process, const QString& command) const;

  MainWindow ui_;
  CtcScenario ctcScenario_;
  QVector<QObject*> subControllers_;
};

} // namespace ctcWizard
