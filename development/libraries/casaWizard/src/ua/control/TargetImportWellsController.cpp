//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TargetImportWellsController.h"

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
   TargetImportController(predictionTargetManager,
                          parent),
   m_calibrationTargetManager(calibrationTargetManager),
   m_targetImportDialogWells(new TargetImportDialogWells())
{
   connect(m_targetImportDialogWells, SIGNAL(accepted()), this, SLOT(slotImportAccepted()));

   const QStringList wellNames = calibrationTargetManager.getWellNames();
   m_targetImportDialogWells->updateWellsTable(wellNames);

   const QStringList surfaceNames = predictionTargetManager.validSurfaceNames();
   m_targetImportDialogWells->updateSurfaceTable(surfaceNames);

   m_targetImportDialogWells->exec();
}

TargetImportWellsController::~TargetImportWellsController(){
   delete m_targetImportDialogWells;
}

void TargetImportWellsController::slotImportPredictionTargets()
{
   TargetInputInfoFromWells targetInputInfo( m_calibrationTargetManager,
                                             m_targetImportDialogWells->wellSelectionStates(),
                                             m_targetImportDialogWells->surfaceSelectionStates(),
                                             m_targetImportDialogWells->temperatureTargetsSelected(),
                                             m_targetImportDialogWells->vreTargetsSelected(),
                                             m_targetImportDialogWells->depthInput());

   PredictionTargetCreator creator(targetInputInfo, getPredictionTargetManager());
   creator.createTargets();
}

} // namespace ua

} // namespace casaWizard
