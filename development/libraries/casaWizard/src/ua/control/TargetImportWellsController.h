//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "view/TargetImportDialogWells.h"

#include <QObject>

namespace casaWizard
{

class CalibrationTargetManager;

namespace ua
{

class PredictionTargetManager;

class TargetImportWellsController: public QObject
{
   Q_OBJECT

public:
   TargetImportWellsController(const CalibrationTargetManager& calibrationTargetManager
                               ,PredictionTargetManager& predictionTargetManager
                               ,QObject* parent = nullptr);
private slots:
   void slotClearAndWritePredictionTargets();
   void slotImportAccepted();
   void slotImportPredictionTargets();

private:
   const CalibrationTargetManager& m_calibrationTargetManager;
   PredictionTargetManager& m_predictionTargetManager;
   TargetImportDialogWells m_targetImportDialogWells;
};

} // namespace ua

} // namespace casaWizard


