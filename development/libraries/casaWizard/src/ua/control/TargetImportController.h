//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>

namespace casaWizard
{

namespace ua
{

class PredictionTargetManager;

class TargetImportController : public QObject
{
   Q_OBJECT

public:
   TargetImportController(PredictionTargetManager& predictionTargetManager,
                          QObject* parent = nullptr);

   PredictionTargetManager& getPredictionTargetManager() const;

private slots:
   virtual void slotImportPredictionTargets() = 0;
   void slotClearAndWritePredictionTargets();
   void slotImportAccepted();

private:
   PredictionTargetManager& m_predictionTargetManager;
};

} // namespace ua

} // namespace casaWizard
