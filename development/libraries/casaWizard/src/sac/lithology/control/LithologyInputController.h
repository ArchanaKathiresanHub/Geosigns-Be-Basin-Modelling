//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/SacInputController.h"
#include "model/LithologyScenario.h"
#include "view/LithologyInputTab.h"

#include <QObject>
#include <QVector>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithofractionController;

class LithologyInputController : public SacInputController
{
   Q_OBJECT

public:
   LithologyInputController(LithologyInputTab* inputTab,
                            LithologyScenario& casaScenario,
                            ScriptRunController& scriptRunController,
                            QObject* parent);

private slots:
   void slotPushButtonSelectProject3dClicked() final;
   void slotUpdateTabGUI(int tabID) final;

private:
   LithologyScenario& scenario() final;
   LithologyScenario& scenario() const final;

   LithologyInputTab* inputTab();
   const LithologyInputTab* inputTab() const;

   void refreshGUI() final;
   std::unique_ptr<SACScript> optimizationScript(const QString& baseDirectory,
                                                 bool doOptimization = true) final;
   void readOptimizedResults() final;

   LithologyScenario& m_scenario;
   LithologyInputTab* m_inputTab;
   LithofractionController* m_lithofractionController;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
