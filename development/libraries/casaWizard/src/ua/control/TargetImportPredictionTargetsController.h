//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <control/TargetImportController.h>

namespace casaWizard
{

namespace ua
{

class TargetImportDialogAscii;

class TargetImportPredictionTargetsController : public TargetImportController
{
   Q_OBJECT

public:
   TargetImportPredictionTargetsController(PredictionTargetManager& predictionTargetManager,
                                           QObject* parent = nullptr);
   ~TargetImportPredictionTargetsController() override;

private slots:
   void slotImportPredictionTargets() override;
   bool slotPushSelectFileClicked();

private:
   TargetImportDialogAscii* m_targetImportDialogAscii;
};

} // namespace ua

} // namespace casaWizard
