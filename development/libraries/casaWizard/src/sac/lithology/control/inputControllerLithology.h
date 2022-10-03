//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/SacInputController.h"
#include "model/sacLithologyScenario.h"

#include <QObject>
#include <QVector>

namespace casaWizard
{

namespace sac
{

class LithofractionController;

namespace lithology
{

class InputTab;

class InputControllerLithology : public SacInputController
{
   Q_OBJECT

public:
   InputControllerLithology(InputTab* inputTab,
                            SacLithologyScenario& casaScenario,
                            ScriptRunController& scriptRunController,
                            QObject* parent);

private slots:
   bool slotPushButtonSelectProject3dClicked() override;
   void slotUpdateTabGUI(int tabID) override;

private:
   void refreshGUI() override;
   SacLithologyScenario& scenario() override;
   SacLithologyScenario& scenario() const override;
   std::unique_ptr<SACScript> optimizationScript(const QString& baseDirectory, bool doOptimization = true) override;
   void readOptimizedResults() override;

   SacLithologyScenario& casaScenario_;
   LithofractionController* lithofractionController_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
