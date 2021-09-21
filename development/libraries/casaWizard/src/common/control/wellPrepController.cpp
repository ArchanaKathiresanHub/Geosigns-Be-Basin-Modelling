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
#include "model/dtToTwoWayTimeConverter.h"
#include "model/input/cmbMapReader.h"
#include "model/logger.h"
#include "model/oneDModelDataExtractor.h"
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
  connect(wellPrepTab->buttonDTtoTWT(), SIGNAL(clicked()), this, SLOT(slotConvertDTToTWT()));
  connect(wellPrepTab->openDataFileButton(), SIGNAL(clicked()), this, SLOT(slotPushSelectCalibrationClicked()));
  connect(calibrationTargetController_, SIGNAL(wellSelectionChanged()), this, SLOT(slotWellSelectionChanged()));
  connect(wellPrepTab->buttonCropBasement(), SIGNAL(clicked()), this, SLOT(slotRemoveDataBelowBasementAndAboveMudline()));
  connect(wellPrepTab->buttonCropOutline(), SIGNAL(clicked()), this, SLOT(slotRemoveWellsOutOfBasinOutline()));

}

void WellPrepController::slotUpdateTabGUI(int tabID)
{
  if (!checkTabID(tabID))
  {
    return;
  }

  checkEnabledStateButtons();
}

void WellPrepController::slotConvertDTToTWT()
{
  Logger::log() << "Converting SonicSlowness to TwoWayTime..." << Logger::endl();
  CalibrationTargetManager& calibrationManager = calibrationTargetController_->calibrationTargetManager();
  const DTToTwoWayTimeConverter converter;

  const OneDModelDataExtractor extractor(casaScenario_);
  const ModelDataPropertyMap data = extractor.extract("TwoWayTime");

  const QString convertedTWTName = "TWT_FROM_DT";
  calibrationManager.removeCalibrationTargetsFromActiveWellsWithPropertyUserName(convertedTWTName);

  for (const Well* well : calibrationManager.wells())
  {
    if (well->isActive())
    {
      QStringList properties;
      const QVector<QVector<const CalibrationTarget*>> targetsInWell = calibrationManager.extractWellTargets(properties, well->id());

      const QString SonicSlownessUserName = calibrationManager.getSonicSlownessUserNameForConversion(properties);

      if (SonicSlownessUserName == "")
      {
        Logger::log() << "Well " << well->name() << " does not have any SonicSlowness data to convert to TwoWayTime." << Logger::endl();
        continue;
      }

      const QVector<const CalibrationTarget*> dtTargets = targetsInWell[properties.indexOf(SonicSlownessUserName)];
      const std::pair<DepthVector, PropertyVector> modelDataForWell = data.at(well->name());

      std::vector<double> DT;
      std::vector<double> depth;
      for (const CalibrationTarget* target : dtTargets)
      {
        DT.push_back(target->value());
        depth.push_back(target->z());
      }

      double TwoWayTimeJustAboveWellData = 0.0;
      for (int i = 1; i < modelDataForWell.first.size(); i++)
      {
        if (modelDataForWell.first[i] > depth[0])
        {
          TwoWayTimeJustAboveWellData = modelDataForWell.second[i-1];
        }
      }

      const std::vector<double> TWT = converter.convertToTWT(DT, depth, modelDataForWell.second, modelDataForWell.first);

      for (int i = 0; i < TWT.size(); i++)
      {
        const QString targetName(QString("TwoWayTime") + "(" +
                                 QString::number(well->x(),'f',1) + "," +
                                 QString::number(well->y(),'f',1) + "," +
                                 QString::number(depth[i],'f',1) + ")");
        calibrationManager.addCalibrationTarget(targetName, convertedTWTName,
                                                well->id(), depth[i], TWT[i]);
      }
    }
  }

  calibrationManager.addToMapping("TWT_FROM_DT", "TwoWayTime");
  calibrationManager.updateObjectiveFunctionFromTargets();

  refreshGUI();
  Logger::log() << "Done!" << Logger::endl();
}

void WellPrepController::slotWellSelectionChanged()
{
  checkEnabledStateButtons();
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
