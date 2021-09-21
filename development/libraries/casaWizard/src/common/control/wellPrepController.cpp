//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellPrepController.h"

#include "scriptRunController.h"
#include "calibrationTargetWellPrepController.h"

#include "control/importWellPopupController.h"
#include "control/loadTargetsThread.h"
#include "model/casaScenario.h"
#include "model/input/cmbMapReader.h"
#include "model/logger.h"
#include "model/wellValidator.h"

#include "view/wellPrepTab.h"
#include "view/calibrationTargetTable.h"
#include "view/components/emphasisbutton.h"

#include <QFileDialog>
#include <QPushButton>


namespace casaWizard
{

WellPrepController::WellPrepController(WellPrepTab* wellPrepTab,
                                       CasaScenario& casaScenario,
                                       ScriptRunController& scriptRunController,
                                       QObject* parent):
  QObject(parent),  
  wellPrepTab_{wellPrepTab},
  casaScenario_{casaScenario},
  scriptRunController_{scriptRunController},
  calibrationTargetController_{new CalibrationTargetWellPrepController(wellPrepTab->calibrationTargetTable(), casaScenario, this)},
  importWellPopupController_{new ImportWellPopupController(wellPrepTab->importWellPopup(), this)},
  importingPopup_{}
{
  importingPopup_.setIcon(QMessageBox::Icon::Information);
  importingPopup_.setWindowTitle("Importing");
  importingPopup_.setText("Please wait while the wells are imported and validated.");
  importingPopup_.setStandardButtons(nullptr);

  connect(wellPrepTab->buttonSelectAll(), SIGNAL(clicked()), calibrationTargetController_, SLOT(slotSelectAllWells()));
  connect(wellPrepTab->buttonDeselectAll(), SIGNAL(clicked()), calibrationTargetController_, SLOT(slotClearWellSelection()));

  connect(wellPrepTab->openDataFileButton(), SIGNAL(clicked()), this, SLOT(slotPushSelectCalibrationClicked()));
  connect(wellPrepTab->buttonCropBasement(), SIGNAL(clicked()), this, SLOT(slotRemoveDataBelowBasementAndAboveMudline()));
  connect(wellPrepTab->buttonCropOutline(), SIGNAL(clicked()), this, SLOT(slotRemoveWellsOutOfBasinOutline()));
}

void WellPrepController::slotUpdateTabGUI(int tabID)
{
  if (!checkTabID(tabID))
  {
    return;
  }

  refreshGUI();
}

void WellPrepController::refreshGUI()
{
  const bool hasProject = !casaScenario_.project3dPath().isEmpty();

  wellPrepTab_->buttonCropOutline()->setEnabled(hasProject);
  wellPrepTab_->buttonCropBasement()->setEnabled(hasProject);
  wellPrepTab_->buttonToSAC()->setEnabled(hasProject);

  emit signalRefreshChildWidgets();
}

void WellPrepController::slotPushSelectCalibrationClicked()
{
  QString fileName = QFileDialog::getOpenFileName(wellPrepTab_,
                                                  "Select calibration targets",
                                                  "",
                                                  "Spreadsheet (*.xlsx)");
  if (fileName == "")
  {
    return;
  }

  CalibrationTargetManager& temporaryImportCalibrationTargetManager = importWellPopupController_->importCalibrationTargetManager();
  temporaryImportCalibrationTargetManager.clear();

  importOnSeparateThread(temporaryImportCalibrationTargetManager, fileName);
  importingPopup_.exec();

  CalibrationTargetManager& ctManager = casaScenario_.calibrationTargetManagerWellPrep();
  temporaryImportCalibrationTargetManager.copyMappingFrom(ctManager);

  if (importWellPopupController_->executeImportWellPopup() != QDialog::Accepted)
  {
    return;
  }

  ctManager.appendFrom(temporaryImportCalibrationTargetManager);

  if (ctManager.objectiveFunctionManager().indexOfCauldronName("Velocity") != -1)
  {
    QMessageBox velocityDisabled(QMessageBox::Icon::Information,
                          "Velocity calibration data disabled",
                          "It is not possible to optimize using velocity calibration data. If you want to use the velocity data, first convert to SonicSlowness (DT)",
                          QMessageBox::Ok);
    velocityDisabled.exec();
  }

  refreshGUI();
}

void WellPrepController::importOnSeparateThread(CalibrationTargetManager& temporaryImportCalibrationTargetManager, const QString& fileName)
{
  LoadTargetsThread* loadTargetsThread = new LoadTargetsThread(casaScenario_, temporaryImportCalibrationTargetManager, fileName, this);
  loadTargetsThread->start();
  connect (loadTargetsThread, &LoadTargetsThread::finished, this, &WellPrepController::slotCloseWaitingDialog);
  connect (loadTargetsThread, &LoadTargetsThread::finished, loadTargetsThread, &QObject::deleteLater);
}

void WellPrepController::slotCloseWaitingDialog()
{
  importingPopup_.done(0);
}

void WellPrepController::slotRemoveDataBelowBasementAndAboveMudline()
{
  Logger::log() << "Starting removal of data below the basement or above the mudline of the domain model..." << Logger::endl();

  CalibrationTargetManager& calibrationManager = casaScenario_.calibrationTargetManager();
  CMBMapReader mapReader;
  mapReader.load(casaScenario_.project3dPath().toStdString());
  const QStringList surfaceNames = casaScenario_.projectReader().surfaceNames();
  const QString basementGridName = casaScenario_.projectReader().getDepthGridName(surfaceNames.size() - 1);
  const QString mudlineGridName = casaScenario_.projectReader().getDepthGridName(0);

  std::vector<double> basementDepthsAtActiveWellLocations;
  std::vector<double> mudlineDepthsAtActiveWellLocations;

  for (const Well* well : calibrationManager.activeWells())
  {
    if (well->isActive())
    {
      basementDepthsAtActiveWellLocations.push_back(mapReader.getValue(well->x(), well->y(), basementGridName.toStdString()));
      mudlineDepthsAtActiveWellLocations.push_back(mapReader.getValue(well->x(), well->y(), mudlineGridName.toStdString()));
    }
  }

  calibrationManager.removeDataOutsideModelDepths(basementDepthsAtActiveWellLocations, mudlineDepthsAtActiveWellLocations);

  Logger::log() << "Done!" << Logger::endl();
}

void WellPrepController::slotRemoveWellsOutOfBasinOutline()
{
  Logger::log() << "Starting removal of wells outside of the basin outline model..." << Logger::endl();

  casaScenario_.calibrationTargetManagerWellPrep().removeWellsOutsideBasinOutline(casaScenario_.project3dPath().toStdString(),
                                                                                  casaScenario_.projectReader().getDepthGridName(0).toStdString());

  refreshGUI();

  Logger::log() << "Done!" << Logger::endl();
}

} // namespace casaWizard
