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
#include "model/PredictionTargetCreator.h"
#include "model/TargetInputInfoFromWells.h"

#include "view/TargetImportDialogWells.h"

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
   m_predictionTargetManager(predictionTargetManager),
   m_targetImportDialogWells(new TargetImportDialogWells())
{
   connect(m_targetImportDialogWells, SIGNAL(accepted()), this, SLOT(slotImportAccepted()));

   const QStringList wellNames = calibrationTargetManager.getWellNames();
   m_targetImportDialogWells->updateWellsTable(wellNames);

   const QStringList surfaceNames = predictionTargetManager.validSurfaceNames();
   m_targetImportDialogWells->updateSurfaceTable(surfaceNames);

   m_targetImportDialogWells->exec();
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
   TargetInputInfoFromWells targetInputInfo( m_calibrationTargetManager,
                                             m_targetImportDialogWells->wellSelectionStates(),
                                             m_targetImportDialogWells->surfaceSelectionStates(),
                                             m_targetImportDialogWells->temperatureTargetsSelected(),
                                             m_targetImportDialogWells->vreTargetsSelected(),
                                             m_targetImportDialogWells->depthInput());

   PredictionTargetCreator creator(targetInputInfo, m_predictionTargetManager);
   creator.createTargets();
}

void TargetImportWellsController::slotClearAndWritePredictionTargets()
{
   m_predictionTargetManager.clear();
   slotImportPredictionTargets();
}

} // namespace ua

} // namespace casaWizard
