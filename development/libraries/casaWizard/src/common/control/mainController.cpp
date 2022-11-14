//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mainController.h"

#include "model/casaScenario.h"
#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/script/runScript.h"
#include "ScriptRunController.h"
#include "view/mainWindow.h"
#include "view/menuBar.h"
#include "view/logDisplay.h"

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

   connect(&Logger::log(), SIGNAL(logSignal(const QString&)), logReceiver_, SLOT(log(const QString&)));
   connect(scriptRunController_,  SIGNAL(readyReadStandardOutput()), this, SLOT(logMessage()));
   connect(scriptRunController_, SIGNAL(runStarted()), this, SLOT(slotLockTabs()));
   connect(scriptRunController_, SIGNAL(runEnded()), this, SLOT(slotUnlockTabs()));
   connect(mainWindow().tabWidget(), SIGNAL(currentChanged(int)), this, SLOT(slotTabSwitch(int)));

   connect(mainWindow().menu()->actionNew(),  SIGNAL(triggered()), this, SLOT(slotNew()));
   connect(mainWindow().menu()->actionSave(), SIGNAL(triggered()), this, SLOT(slotSave()));
   connect(mainWindow().menu()->actionOpen(), SIGNAL(triggered()), this, SLOT(slotOpen()));
   connect(mainWindow().menu()->actionExit(), SIGNAL(triggered()), this, SLOT(slotExit()));

   connect(mainWindow().menu()->actionExpertUser(), SIGNAL(toggled(bool)), this, SLOT(slotExpertUser(bool)));
   connect(mainWindow().menu()->actionReload1Dresults(), SIGNAL(triggered()), SIGNAL(signalReload1Ddata()));

   connect(mainWindow().logDisplay(), SIGNAL(resizeLog(int)), &mainWindow(), SLOT(slotResizeLog(int)));

   showFirstTab();
}

void MainController::showFirstTab()
{
   mainWindow().tabWidget()->setCurrentIndex(0);
   emit signalUpdateTabGUI(0);

   refreshGUI();
}

void MainController::slotNew()
{
   scenario().clear();
   showFirstTab();
   logReceiver_->clear();

   Logger::log() << "Clearing the scenario" << Logger::endl();
}

void MainController::slotSave()
{
   QString fileName = QFileDialog::getSaveFileName(&mainWindow(), "Save scenario", scenario().workingDirectory(), ".dat (*.dat)");

   if (fileName.isEmpty())
   {
      return;
   }
   if (!fileName.endsWith(".dat"))
   {
      fileName.append(".dat");
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

   scenario().clear();
   logReceiver_->clear();

   QDir folderLocation = QFileInfo(fileName).dir();
   folderLocation.cdUp();
   scenario().setWorkingDirectory( folderLocation.path() );

   QString file{fileName};
   try
   {
      ScenarioReader reader{file};
      scenario().readFromFile(reader);
   }
   catch (const std::exception& e)
   {
      Logger::log() << "Failed to read saved file: " << e.what() << Logger::endl();
      scenario().clear();
      showFirstTab();
      return;
   }

   emit signalProjectOpened();

   showFirstTab();
   Logger::log() << "Reading from file " << file << Logger::endl();
}

void MainController::slotExit()
{
   mainWindow().close();
}

void MainController::slotExpertUser(bool isExpertUser)
{
   scenario().setExpertUser(isExpertUser);
}

void MainController::slotLockTabs()
{
   mainWindow().tabWidget()->setDisabled(true);
   mainWindow().menu()->setDisabled(true);
}

void MainController::slotUnlockTabs()
{
   mainWindow().tabWidget()->setEnabled(true);
   mainWindow().menu()->setEnabled(true);
}

void MainController::refreshGUI()
{
   mainWindow().menu()->setActionExpertUser(scenario().expertUser());
}

ScriptRunController& MainController::scriptRunController()
{
   return *scriptRunController_;
}

void MainController::logMessage()
{
   logReceiver_->log(QString(scriptRunController_->processOutput()));
}

void MainController::slotTabSwitch(int tabIndex)
{
   emit signalUpdateTabGUI(tabIndex);
}

void MainController::slotSwitchToTab(int tabIndex)
{
   mainWindow().tabWidget()->setCurrentIndex(tabIndex);
}

} // namespace casaWizard
