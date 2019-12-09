// Base class for a main controller of a wizard
#pragma once

#include "logReceiver.h"

#include <QObject>

namespace casaWizard
{

class CasaScenario;
class MainWindow;
class ScriptRunController;

class MainController : public QObject
{
  Q_OBJECT

public:
  explicit MainController(QObject* parent = 0);
  virtual ~MainController() = default;

  ScriptRunController& scriptRunController();
  virtual MainWindow& mainWindow() = 0;
  virtual CasaScenario& scenario() = 0;
  void constructWindow(LogReceiver* logReceiver);

private slots:
  void slotNew();
  void slotOpen();
  void slotSave();
  void slotExit();
  void slotExpertUser(bool);

  void logMessage();
  void slotTabSwitch(int tabIndex);

  void slotRefresh();

signals:
  void signalRefresh(int tabIndex = 0);
  void signalEnableDisableWorkflowTabs(int tabIndex, bool hasLogMessage = true);
  void signalResestToStartingStage();
  void signalRefreshAfterOpen();
  void signalProjectOpened();

private:
  void emitAllRefreshSignals();
  void emitAllEnableDisableSignals(const bool hasLogMessage = true);

  LogReceiver* logReceiver_;
  ScriptRunController* scriptRunController_;
};

} // casaWizard
