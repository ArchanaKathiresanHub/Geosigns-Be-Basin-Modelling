//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupController.h"

#include "loadTargetsThread.h"
#include "model/calibrationTarget.h"
#include "model/casaScenario.h"
#include "model/input/calibrationTargetCreator.h"
#include "view/importWellPopup.h"

#include <QMessageBox>
#include <QSet>
#include <QString>
#include <QTableWidget>

namespace casaWizard
{

ImportWellPopupController::ImportWellPopupController(QObject* parent, CasaScenario& casaScenario):
  QObject(parent),
  importCalibrationTargetManager_{},
  casaScenario_{casaScenario},
  waitingDialog_{},
  waitingDialogNeeded_{true}
{
  waitingDialog_.setIcon(QMessageBox::Icon::Information);
  waitingDialog_.setStandardButtons(nullptr);
}

void ImportWellPopupController::addNewMapping()
{
  const QMap<QString, QString> newMapping = importWellPopup()->getCurrentMapping();
  for (const QString& key : newMapping.keys())
  {
    if (newMapping[key] != "Depth")
    {
      importCalibrationTargetManager_.addToMapping(key, newMapping[key]);
    }
  }
}

CalibrationTargetManager& ImportWellPopupController::importCalibrationTargetManager()
{
  return importCalibrationTargetManager_;
}

void ImportWellPopupController::importWells(const QString& fileName)
{
  importCalibrationTargetManager_.clear();

  if (!importWellsToCalibrationTargetManager(fileName))
  {
    return;
  }

  const QMap<QString, QString>& mapping = importCalibrationTargetManager_.userNameToCauldronNameMapping();
  for (const QString& key : mapping.keys())
  {
    QString cauldronName = mapping[key];
    // User name is the Cauldron name, or "TWT_FROM_DT" or "DT_FROM_VP"
    importCalibrationTargetManager_.renameUserPropertyNameInWells(key, cauldronName);

    // Real Cauldron property name, so remove the fake ones
    if (cauldronName == "TWT_FROM_DT") cauldronName="TwoWayTime";
    if (cauldronName == "DT_FROM_VP")  cauldronName="SonicSlowness";
    importCalibrationTargetManager_.addToMapping(mapping[key], cauldronName);
  }

  CalibrationTargetManager& ctManager = casaScenario_.calibrationTargetManagerWellPrep();
  ctManager.appendFrom(importCalibrationTargetManager_);
}

void ImportWellPopupController::importOnSeparateThread(CalibrationTargetCreator& calibrationTargetCreator)
{
  waitingDialogNeeded_ = true;
  LoadTargetsThread* loadTargetsThread = new LoadTargetsThread(calibrationTargetCreator, this);
  connect (loadTargetsThread, &LoadTargetsThread::finished, this, &ImportWellPopupController::slotCloseWaitingDialog);
  connect (loadTargetsThread, &LoadTargetsThread::finished, loadTargetsThread, &QObject::deleteLater);
  loadTargetsThread->start();
  waitingDialog_.setWindowTitle("Importing");
  waitingDialog_.setText("Please wait while the wells are imported and validated.");
  if (waitingDialogNeeded_) waitingDialog_.exec();
}

void ImportWellPopupController::slotCloseWaitingDialog()
{
  waitingDialogNeeded_ = false;
  waitingDialog_.done(0);
}


} // namespace casaWizard
