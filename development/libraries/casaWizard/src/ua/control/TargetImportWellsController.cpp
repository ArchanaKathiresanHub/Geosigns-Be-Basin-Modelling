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
   writePredictionTargets();
}

void TargetImportWellsController::writePredictionTargets()
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

} // namespace ua

} // namespace casaWizard
