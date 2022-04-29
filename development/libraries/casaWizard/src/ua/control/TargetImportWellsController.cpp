//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TargetImportWellsController.h"

#include "model/calibrationTargetManager.h"
#include "model/predictionTargetManager.h"
#include "model/PredictionTargetsFromWellsCreator.h"
#include "model/TargetInputFromWellsInfo.h"

#include <QStringList>
#include <QMessageBox>
#include <QPushButton>

namespace casaWizard
{

namespace ua
{

TargetImportWellsController::TargetImportWellsController(const CalibrationTargetManager& calibrationTargetManager
                                                         ,PredictionTargetManager& predictionTargetManager
                                                         ,QObject* parent):
   QObject(parent),
   m_calibrationTargetManager(calibrationTargetManager),
   m_predictionTargetManager(predictionTargetManager)
{
   connect(&m_targetImportDialogWells, SIGNAL(accepted()), this, SLOT(slotImportAccepted()));

   const QStringList wellNames = calibrationTargetManager.getWellNames();
   m_targetImportDialogWells.updateWellsTable(wellNames);

   const QStringList surfaceNames = predictionTargetManager.validSurfaceNames();
   m_targetImportDialogWells.updateSurfaceTable(surfaceNames);

   m_targetImportDialogWells.exec();
}

void TargetImportWellsController::slotImportAccepted()
{
   if (m_predictionTargetManager.amountAtAge0() > 0)
   {
      QMessageBox overwriteData(QMessageBox::Icon::Information,
                                "The target table already has targets.",
                                "Would you like to overwrite or append the new targets?");
      QPushButton* appendButton = overwriteData.addButton("Append", QMessageBox::RejectRole);
      QPushButton* overwriteButton = overwriteData.addButton("Overwrite", QMessageBox::AcceptRole);
      connect(appendButton, SIGNAL(clicked()), this, SLOT(slotImportPredictionTargets()));
      connect(overwriteButton, SIGNAL(clicked()), this, SLOT(slotClearAndWritePredictionTargets()));
      overwriteData.exec();
      //If the dialog is closed without selecting append or overwrite, no prediction targets are imported.
   }
   else
   {
      slotImportPredictionTargets();
   }
}

void TargetImportWellsController::slotImportPredictionTargets()
{
   TargetInputFromWellsInfo targetInputInfo;

   targetInputInfo.wellSelectionStates = m_targetImportDialogWells.wellSelectionStates();
   targetInputInfo.surfaceSelectionStates = m_targetImportDialogWells.surfaceSelectionStates();
   targetInputInfo.temperatureTargetsSelected = m_targetImportDialogWells.temperatureTargetsSelected();
   targetInputInfo.vreTargetsSelected = m_targetImportDialogWells.vreTargetsSelected();
   targetInputInfo.depthInput = m_targetImportDialogWells.depthInput();

   PredictionTargetsFromWellsCreator::createTargetsFromWellData(targetInputInfo,
                                                                m_calibrationTargetManager,
                                                                m_predictionTargetManager);
}

void TargetImportWellsController::slotClearAndWritePredictionTargets()
{
   m_predictionTargetManager.clear();
   slotImportPredictionTargets();
}

} // namespace ua

} // namespace casaWizard
