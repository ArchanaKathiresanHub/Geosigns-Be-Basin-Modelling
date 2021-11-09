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

#include "control/importWellPopupLASController.h"
#include "control/importWellPopupVSETController.h"
#include "control/importWellPopupXlsxController.h"
#include "control/loadTargetsThread.h"
#include "control/saveTargetsThread.h"
#include "model/casaScenario.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/input/cmbMapReader.h"
#include "model/logger.h"
#include "model/output/calibrationTargetSaver.h"
#include "model/wellValidator.h"

#include "view/calibrationTargetTable.h"
#include "view/components/emphasisbutton.h"
#include "view/userPropertyChoiceCutOffPopup.h"
#include "view/userPropertyChoicePopup.h"
#include "view/wellPrepTab.h"

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
  importWellPopupController_{},
  userPropertyChoicePopup_{new UserPropertyChoicePopup(wellPrepTab_)},
  userPropertyChoiceCutOffPopup_{new UserPropertyChoiceCutOffPopup(wellPrepTab_)},
  waitingDialog_{},
  waitingDialogNeeded_{true}
{
  waitingDialog_.setIcon(QMessageBox::Icon::Information);
  waitingDialog_.setStandardButtons(nullptr);

  connect(wellPrepTab->buttonSelectAll(), SIGNAL(clicked()), calibrationTargetController_, SLOT(slotSelectAllWells()));
  connect(wellPrepTab->buttonDeselectAll(), SIGNAL(clicked()), calibrationTargetController_, SLOT(slotClearWellSelection()));
  connect(wellPrepTab->buttonDeleteSelection(), SIGNAL(clicked()), calibrationTargetController_, SLOT(slotDeleteSelectedWells()));
  connect(wellPrepTab->buttonExportXYascii(), SIGNAL(clicked()), this, SLOT(slotExportXYasciiClicked()));
  connect(wellPrepTab->buttonExport(), SIGNAL(clicked()), this, SLOT(slotPushSaveDataClicked()));
  connect(wellPrepTab->buttonDTtoTWT(), SIGNAL(clicked()), this, SLOT(slotConvertDTtoTWT()));
  connect(wellPrepTab->buttonVPtoDT(), SIGNAL(clicked()), this, SLOT(slotConvertVPToDT()));
  connect(wellPrepTab->openDataFileButton(), SIGNAL(clicked()), this, SLOT(slotPushSelectCalibrationClicked()));
  connect(wellPrepTab->buttonCropBasement(), SIGNAL(clicked()), this, SLOT(slotRemoveDataBelowBasementAndAboveMudline()));
  connect(wellPrepTab->buttonCropOutline(), SIGNAL(clicked()), this, SLOT(slotRemoveWellsOutOfBasinOutline()));

  connect(wellPrepTab->buttonApplySmoothing(), SIGNAL(clicked()), this, SLOT(slotSelectPropertiesForSmoothing()));
  connect(wellPrepTab->buttonApplySubsampling(), SIGNAL(clicked()), this, SLOT(slotSelectPropertiesForSubsampling()));
  connect(wellPrepTab->buttonApplyCutOff(), SIGNAL(clicked()), this, SLOT(slotSelectPropertiesForCutOff()));

  connect(calibrationTargetController_, SIGNAL(wellSelectionChanged()), this, SLOT(slotWellSelectionChanged()));
}

void WellPrepController::slotUpdateTabGUI(int tabID)
{
  if (!checkTabID(tabID))
  {
    return;
  }

  checkEnabledStateButtons();
}

void WellPrepController::slotConvertDTtoTWT()
{
  if (casaScenario_.calibrationTargetManagerWellPrep().activeWells().empty())
  {
    Logger::log() << "No wells selected for the conversion." << Logger::endl();
    return;
  }

  Logger::log() << "Converting SonicSlowness to TwoWayTime..." << Logger::endl();
  CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();

  const std::string iterationFolder = casaScenario_.original1dDirectory().toStdString() + "/"
                                     + casaScenario_.runLocation().toStdString()
                                     + "/Iteration_1/";
  const std::string project3dFilename = casaScenario_.project3dFilename().toStdString();

  calibrationManager.convertDTtoTWT(iterationFolder, project3dFilename);

  refreshGUI();
  Logger::log() << "Done!" << Logger::endl();
}

void WellPrepController::slotConvertVPToDT()
{
  if (casaScenario_.calibrationTargetManagerWellPrep().activeWells().empty())
  {
    Logger::log() << "No wells selected for the conversion." << Logger::endl();
    return;
  }

  Logger::log() << "Converting Velocity to SonicSlowness..." << Logger::endl();
  CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();

  calibrationManager.convertVPtoDT();

  refreshGUI();
  Logger::log() << "Done!" << Logger::endl();
}

void WellPrepController::slotWellSelectionChanged()
{
  checkEnabledStateButtons();
}

void WellPrepController::slotSelectPropertiesForSmoothing()
{  
  const CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
  const QStringList activePropertyUserNames = calibrationManager.activePropertyUserNames();

  userPropertyChoicePopup_->updateTable(activePropertyUserNames);

  connect(userPropertyChoicePopup_, SIGNAL(acceptedClicked()), this, SLOT(slotApplySmoothing()) );
  userPropertyChoicePopup_->exec();
  disconnect(userPropertyChoicePopup_, SIGNAL(acceptedClicked()), this, SLOT(slotApplySmoothing()) );
}

void WellPrepController::slotApplySmoothing()
{
  const double radius = wellPrepTab_->smoothingLength();
  const QStringList selectedProperties = userPropertyChoicePopup_->selectedProperties();

  CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
  calibrationManager.smoothenData(selectedProperties, radius);

  userPropertyChoicePopup_->done(0);
}

void WellPrepController::slotSelectPropertiesForSubsampling()
{
  const CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
  const QStringList activePropertyUserNames = calibrationManager.activePropertyUserNames();

  userPropertyChoicePopup_->updateTable(activePropertyUserNames);

  connect(userPropertyChoicePopup_, SIGNAL(acceptedClicked()), this, SLOT(slotApplySubsampling()) );
  userPropertyChoicePopup_->exec();
  disconnect(userPropertyChoicePopup_, SIGNAL(acceptedClicked()), this, SLOT(slotApplySubsampling()) );
}

void WellPrepController::slotApplySubsampling()
{
  const double length = wellPrepTab_->subsamplingDistance();
  const QStringList selectedProperties = userPropertyChoicePopup_->selectedProperties();

  CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
  calibrationManager.subsampleData(selectedProperties, length);

  userPropertyChoicePopup_->done(0);
}

void WellPrepController::slotSelectPropertiesForCutOff()
{
  const CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
  const QStringList activePropertyUserNames = calibrationManager.activePropertyUserNames();

  userPropertyChoiceCutOffPopup_->updateTable(activePropertyUserNames);

  connect(userPropertyChoiceCutOffPopup_, SIGNAL(acceptedClicked()), this, SLOT(slotApplyCutOff()) );
  userPropertyChoiceCutOffPopup_->exec();
  disconnect(userPropertyChoiceCutOffPopup_, SIGNAL(acceptedClicked()), this, SLOT(slotApplyCutOff()) );
}

void WellPrepController::slotApplyCutOff()
{  
  const PropertiesWithCutOffRanges propertiesWithCutOffRanges = userPropertyChoiceCutOffPopup_->getPropertiesWithCutOffRanges();

  CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
  calibrationManager.applyCutOff(propertiesWithCutOffRanges);

  userPropertyChoiceCutOffPopup_->done(0);
}

bool WellPrepController::allActiveWellsHave1DResults() const
{
  for (const Well* well : calibrationTargetController_->calibrationTargetManager().activeWells())
  {
    QFile successFile(casaScenario_.original1dDirectory() + "/" + casaScenario_.runLocation() + "/Iteration_1/" + well->name() + "/Stage_0.sh.success");
    if (!successFile.exists())
    {
      return false;
    }
  }

  return true;
}

void WellPrepController::refreshGUI()
{
  checkEnabledStateButtons();

  emit signalRefreshChildWidgets();
}

void WellPrepController::checkEnabledStateButtons() const
{
  const bool hasProject = !casaScenario_.project3dPath().isEmpty();

  wellPrepTab_->buttonDTtoTWT()->setEnabled(hasProject && allActiveWellsHave1DResults());

  wellPrepTab_->buttonCropOutline()->setEnabled(hasProject);
  wellPrepTab_->buttonCropBasement()->setEnabled(hasProject);
  wellPrepTab_->buttonToSAC()->setEnabled(hasProject);
}

void WellPrepController::slotPushSelectCalibrationClicked()
{
  QString fileName = QFileDialog::getOpenFileName(wellPrepTab_,
                                                  "Select calibration targets",
                                                  "",
                                                  "Well-data files (*.xlsx *.las *.vs)");
  if (fileName == "")
  {
    return;
  }

  delete importWellPopupController_;
  const int dotPosition = fileName.lastIndexOf('.');
  const QString extension = fileName.mid(dotPosition + 1);

  if (extension.toLower() == "xlsx")
  {
    importWellPopupController_ = new ImportWellPopupXlsxController(this, casaScenario_);
  }
  else if (extension.toLower() == "las")
  {
    importWellPopupController_ = new ImportWellPopupLASController(this, casaScenario_);
  }
  else if (extension.toLower() == "vs")
  {
    importWellPopupController_ = new ImportWellPopupVSETController(this, casaScenario_);
  }

  try
  {
    importWellPopupController_->importWellsToCalibrationTargetManager(fileName, casaScenario_.calibrationTargetManagerWellPrep());
  }
  catch (std::runtime_error e)
  {
    QString message = e.what();

    reportImportError(message);
    return;
  }
  catch (...)
  {
    reportImportError("Unknown error occurred during import");
    return;
  }

  Logger::log() << "Done!" << Logger::endl();

  refreshGUI();
}

void WellPrepController::reportImportError(QString message)
{
  if (message == "") message = "Unknown error occurred";
  QMessageBox box(QMessageBox::Critical, "Import Error",
                  message,
                  QMessageBox::Ok);
  box.exec();
}

void WellPrepController::slotExportXYasciiClicked()
{
  if (casaScenario_.calibrationTargetManagerWellPrep().activeWells().empty())
  {
    Logger::log() << "No wells selected for saving." << Logger::endl();
    return;
  }

  const QString fileName = QFileDialog::getSaveFileName(wellPrepTab_, "Save as", "", "All files (*.*)");
  if (fileName == "")
  {
    return;
  }

  CalibrationTargetSaver saver(casaScenario_.calibrationTargetManagerWellPrep());
  if (saver.saveXYtoASCII(fileName))
  {
    Logger::log() << "Done!" << Logger::endl();
  }
}

void WellPrepController::slotPushSaveDataClicked()
{
  if (casaScenario_.calibrationTargetManagerWellPrep().activeWells().empty())
  {
    Logger::log() << "No wells selected for saving." << Logger::endl();
    return;
  }

  QString fileName = QFileDialog::getSaveFileName(wellPrepTab_, "Save as", "", "Spreadsheet (*.xlsx)");
  if (fileName == "")
  {
    return;
  }

  if (!fileName.endsWith(".xlsx"))
  {
    fileName += ".xlsx";
  }

  waitingDialog_.setWindowTitle("Exporting");
  waitingDialog_.setText("Please wait while the wells are exported to the xlsx file.");
  exportOnSeparateThread(casaScenario_.calibrationTargetManagerWellPrep(), fileName);
  if (waitingDialogNeeded_) waitingDialog_.exec();
  if (waitingDialogNeeded_) waitingDialog_.done(0);
}

void WellPrepController::exportOnSeparateThread(const CalibrationTargetManager& calibrationTargetManager, const QString& fileName)
{
  waitingDialogNeeded_ = true;
  SaveTargetsThread* saveTargetsThread = new SaveTargetsThread(calibrationTargetManager, fileName, this);
  connect (saveTargetsThread, &SaveTargetsThread::finished, this, &WellPrepController::slotCloseWaitingDialog);
  connect (saveTargetsThread, &SaveTargetsThread::finished, saveTargetsThread, &QObject::deleteLater);
  saveTargetsThread->start();
}

void WellPrepController::slotCloseWaitingDialog()
{
  waitingDialogNeeded_ = false;
  waitingDialog_.done(0);
}

void WellPrepController::slotRemoveDataBelowBasementAndAboveMudline()
{
  Logger::log() << "Starting removal of data below the basement or above the mudline of the domain model..." << Logger::endl();

  CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
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
