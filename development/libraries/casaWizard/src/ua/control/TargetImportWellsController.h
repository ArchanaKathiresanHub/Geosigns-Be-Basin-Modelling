//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once
#include <QObject>
#include "control/TargetImportController.h"

namespace casaWizard
{

class CalibrationTargetManager;

namespace ua
{

class TargetImportDialogWells;

class TargetImportWellsController: public TargetImportController
{
   Q_OBJECT

public:
   TargetImportWellsController(const CalibrationTargetManager& calibrationTargetManager,
                               PredictionTargetManager& predictionTargetManager,
                               QObject* parent = nullptr);
   ~TargetImportWellsController() override;

private slots:
   void slotImportPredictionTargets() override;

private:
   const CalibrationTargetManager& m_calibrationTargetManager;
   TargetImportDialogWells* m_targetImportDialogWells;
};

} // namespace ua

} // namespace casaWizard


