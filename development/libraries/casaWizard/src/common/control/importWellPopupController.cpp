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
#include "view/components/ErrorMessageBox.h"

#include <QMessageBox>
#include <QSet>
#include <QString>
#include <QTableWidget>

namespace casaWizard
{

ImportWellPopupController::ImportWellPopupController(QObject* parent, CasaScenario& casaScenario):
  QObject(parent),
  casaScenario_{casaScenario},
  waitingDialog_{}
{
  waitingDialog_.setIcon(QMessageBox::Icon::Information);
  waitingDialog_.setStandardButtons(nullptr);
}

void ImportWellPopupController::importOnSeparateThread(CalibrationTargetCreator& calibrationTargetCreator)
{
  LoadTargetsThread* loadTargetsThread = new LoadTargetsThread(calibrationTargetCreator, this);
  connect (loadTargetsThread, SIGNAL(exceptionThrown(QString)), this, SLOT(slotExceptionThrown(QString)));
  connect (loadTargetsThread, &LoadTargetsThread::finished, this, &ImportWellPopupController::slotCloseWaitingDialog);
  connect (loadTargetsThread, &LoadTargetsThread::finished, loadTargetsThread, &QObject::deleteLater);
  loadTargetsThread->start();
  loadTargetsThread->msleep(100); // Make sure the waiting dialog starts before the thread has finished
  waitingDialog_.setWindowTitle("Importing");
  waitingDialog_.setText("Please wait while the wells are imported and validated.");
  waitingDialog_.exec();
}

void ImportWellPopupController::slotCloseWaitingDialog()
{
  waitingDialog_.done(0);
}

void ImportWellPopupController::slotExceptionThrown(QString message)
{
   messageBox::showErrorMessageBox(message);
}


} // namespace casaWizard
