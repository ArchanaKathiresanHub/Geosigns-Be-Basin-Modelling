#include "controller.h"

#include "control/doeController.h"
#include "control/logDisplayController.h"
#include "control/qcController.h"
#include "control/targetController.h"
#include "control/mcmcController.h"
#include "control/correlationController.h"
#include "model/output/runCaseSetFileManager.h"
#include "view/menuBarUA.h"
#include "view/uaTabIDs.h"

#include "model/logger.h"
#include "model/input/cmbProjectReader.h"

#include <QMessageBox>

namespace casaWizard
{

namespace ua
{

Controller::Controller() :
  MainController(),
  ui_{},
  scenario_{std::unique_ptr<ProjectReader>(new CMBProjectReader())},
  doeController_{new DoEcontroller{ui_.doeTab(), scenario_, scriptRunController(), this}},
  targetController_{new TargetController{ui_.targetTab(), scenario_, this}},
  qcController_{new QCController{ui_.qcTab(), scenario_, scriptRunController(), this}},
  mcmcController_{new MCMCController{ui_.mcmcTab(), scenario_, scriptRunController(), this}},
  correlationController_{new CorrelationController{ui_.correlationsTab(), scenario_, scriptRunController(), this}}
{
  const MenuBarUA* menuBarUA = ui_.menuUA();
  connect(menuBarUA->actionRemoveDoeData(),    SIGNAL(triggered()), this, SLOT(slotPopupRemoveDoeData()));
  connect(menuBarUA->actionRemoveDoeDataAll(), SIGNAL(triggered()), this, SLOT(slotPopupRemoveDoeDataAll()));

  connect(this, SIGNAL(signalProjectOpened()), doeController_, SLOT(slotUpdateIterationDir()));

  connect(this, SIGNAL(signalUpdateTabGUI(int)), doeController_,         SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), targetController_,      SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), qcController_,          SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), mcmcController_,        SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), correlationController_, SLOT(slotUpdateTabGUI(int)));

  ui_.show();
  constructWindow(new LogDisplayController(ui_.logDisplay(), this));
}

MainWindow& Controller::mainWindow()
{
  return ui_;
}

CasaScenario& Controller::scenario()
{
  return scenario_;
}

void Controller::slotPopupRemoveDoeData()
{
  RunCaseSetFileManager& runCaseSetFileManager = scenario_.runCaseSetFileManager();

  if (runCaseSetFileManager.isIterationDirEmpty())
  {
    Logger::log() << "No DoE data found! Run Casa first" << Logger::endl();
    return;
  }

  if (runCaseSetFileManager.isIterationDirDeleted(scenario_.project3dPath()))
  {
    Logger::log() << "DoE data has already been deleted!" << Logger::endl();
    return;
  }

  QMessageBox messageBox;
  messageBox.setText("Removing DoE data: this will release " + QString::number(runCaseSetFileManager.iterationDirFilesSize(), 'f', 1) + "MB of disk space!");
  messageBox.setInformativeText("Are you sure you want to continue?");
  messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  messageBox.setDefaultButton(QMessageBox::No);

  if (messageBox.exec() == QMessageBox::No)
  {
    return;
  }

  if (!scenario_.isStageComplete(StageTypesUA::qc))
  {
    QMessageBox warningBox;
    warningBox.setText("Target values are not extracted from DoE data yet (and QC stage is not complete)!");
    warningBox.setInformativeText("Do you still want to continue?");
    warningBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    warningBox.setDefaultButton(QMessageBox::No);
    if ( warningBox.exec() == QMessageBox::No)
    {
      return;
    }
  }

  if (!runCaseSetFileManager.removeIterationDir())
  {
    Logger::log() << "Could not delete the doe data (or no data found to be deleted)!";
  }
  else
  {
    Logger::log() << "Removed directory " << runCaseSetFileManager.iterationDirName() << " with DoE data!" << Logger::endl();
  }

  showFirstTab();
}

void Controller::slotPopupRemoveDoeDataAll()
{
  RunCaseSetFileManager& runCaseSetFileManager = scenario_.runCaseSetFileManager();

  if (scenario_.project3dPath().isEmpty())
  {
    Logger::log() << "No DoE data found! Select a project3d file first to set the path." << Logger::endl();
    return;
  }

  QMessageBox messageBox;
  messageBox.setText("This will remove all DoE data (also from previous runs), "
                     + QString::number(runCaseSetFileManager.allIterationDirsFilesSize(scenario_.project3dPath()), 'f', 1) + "MB of disk space will be released!");
  messageBox.setInformativeText("Are you sure you want to continue?");
  messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  messageBox.setDefaultButton(QMessageBox::No);
  int ret = messageBox.exec();

  if (ret == QMessageBox::Yes)
  {
    if (!runCaseSetFileManager.removeAllIterationDirs(scenario_.project3dPath()))
    {
      Logger::log() << "Could not delete the DoE data (or no data found to be deleted)!" << Logger::endl();
    }
    else
    {
      Logger::log() << "Removed all DoE data!" << Logger::endl();
    }
  }

  showFirstTab();
}

} // namespace ua

} // namespace casaWizard
