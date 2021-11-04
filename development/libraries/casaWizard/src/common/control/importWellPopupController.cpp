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

void ImportWellPopupController::importWells(const QString& fileName, CalibrationTargetManager& calibrationTargetManager)
{
  importCalibrationTargetManager_.clear();

  if (!importWellsToCalibrationTargetManager(fileName))
  {
    return;
  }

  calibrationTargetManager.appendFrom(importCalibrationTargetManager_);
}

void ImportWellPopupController::importOnSeparateThread(CalibrationTargetCreator& calibrationTargetCreator)
{
  waitingDialogNeeded_ = true;
  LoadTargetsThread* loadTargetsThread = new LoadTargetsThread(calibrationTargetCreator, this);
  connect (loadTargetsThread, SIGNAL(exceptionThrown(QString)), this, SLOT(slotExceptionThrown(QString)));
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

void ImportWellPopupController::slotExceptionThrown(QString exception)
{
  throw std::runtime_error(exception.toStdString());
}


} // namespace casaWizard
