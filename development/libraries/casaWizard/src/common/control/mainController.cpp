#include "mainController.h"

#include "model/casaScenario.h"
#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/script/runScript.h"
#include "scriptRunController.h"
#include "view/mainWindow.h"
#include "view/menuBar.h"

#include <QAction>
#include <QDir>
#include <QFileDialog>

namespace casaWizard
{

MainController::MainController(QObject* parent) :
  QObject(parent),
  logReceiver_{nullptr},
  scriptRunController_{new ScriptRunController(this)}
{
}

void MainController::constructWindow(LogReceiver* logReceiver)
{
  logReceiver_ = logReceiver;

  connect(this, SIGNAL(signalRefresh()), SLOT(slotRefresh()));

  connect(&Logger::log(), SIGNAL(logSignal(const QString&)), logReceiver_, SLOT(log(const QString&)));
  connect(scriptRunController_,  SIGNAL(readyReadStandardOutput()), this, SLOT(logMessage()));
  connect(mainWindow().tabWidget(), SIGNAL(currentChanged(int)), this, SLOT(slotTabSwitch(int)));

  connect(mainWindow().menu()->actionNew(),  SIGNAL(triggered()), this, SLOT(slotNew()));
  connect(mainWindow().menu()->actionSave(), SIGNAL(triggered()), this, SLOT(slotSave()));
  connect(mainWindow().menu()->actionOpen(), SIGNAL(triggered()), this, SLOT(slotOpen()));
  connect(mainWindow().menu()->actionExit(), SIGNAL(triggered()), this, SLOT(slotExit()));

  connect(mainWindow().menu()->actionExpertUser(), SIGNAL(toggled(bool)), this, SLOT(slotExpertUser(bool)));

  emitAllRefreshSignals();
  signalEnableDisableWorkflowTabs(0, false);
}

void MainController::slotNew()
{
  scenario().clear();
  emit signalResestToStartingStage();
  emitAllRefreshSignals();
  emitAllEnableDisableSignals();
  mainWindow().tabWidget()->setCurrentIndex(0);
  logReceiver_->clear();
  Logger::log() << "Clearing the scenario" << Logger::endl();
}

void MainController::slotSave()
{
  const QString fileName = QFileDialog::getSaveFileName(&mainWindow(), "Save scenario", scenario().workingDirectory(), ".dat (*.dat)");

  if (fileName.isEmpty())
  {
    return;
  }

  QString file{fileName};
  Logger::log() << "Saving to file " << file << Logger::endl();
  ScenarioWriter writer{file};
  scenario().writeToFile(writer);
  writer.close();
}

void MainController::slotOpen()
{
  const QString fileName = QFileDialog::getOpenFileName(&mainWindow(), "Open scenario", scenario().workingDirectory(), ".dat (*.dat)");

  if (fileName.isEmpty())
  {
    return;
  }

  QString file{fileName};
  ScenarioReader reader{file};
  scenario().readFromFile(reader);

  QDir folderLocation = QFileInfo(fileName).absoluteDir();
  folderLocation.cdUp();
  scenario().setWorkingDirectory(folderLocation.absolutePath());

  emit signalResestToStartingStage();
  emitAllRefreshSignals();
  emitAllEnableDisableSignals(false);
  Logger::log() << "Reading from file " << file << Logger::endl();

  emit signalRefreshAfterOpen();
}

void MainController::slotExit()
{
  mainWindow().close();
}

void MainController::slotExpertUser(bool isExpertUser)
{
  scenario().setExpertUser(isExpertUser);
}

void MainController::slotRefresh()
{
  mainWindow().menu()->setActionExpertUser(scenario().expertUser());
}

ScriptRunController& MainController::scriptRunController()
{
  return *scriptRunController_;
}

void MainController::logMessage()
{
  logReceiver_->log(QString(scriptRunController_->readAllStandardOutput()));
}

void MainController::slotTabSwitch(int tabIndex)
{
  emit signalRefresh(tabIndex);
  emit signalEnableDisableWorkflowTabs(tabIndex);
}

void MainController::emitAllRefreshSignals()
{
  const QTabWidget* tabWidget{mainWindow().tabWidget()};
  for (int i = 0; i < tabWidget->count(); ++i)
  {
    emit signalRefresh(i);
  }
}

void MainController::emitAllEnableDisableSignals(const bool hasLogMessage)
{
  const QTabWidget* tabWidget{mainWindow().tabWidget()};
  for (int i = 0; i < tabWidget->count(); ++i)
  {
    emit signalEnableDisableWorkflowTabs(i, hasLogMessage);
  }
}

} // namespace casaWizard
