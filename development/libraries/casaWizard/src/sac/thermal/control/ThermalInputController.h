//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/SacInputController.h"
#include "model/ThermalScenario.h"
#include "view/ThermalInputTab.h"
#include <memory>

namespace casaWizard
{

namespace sac
{

class SACScript;

namespace thermal
{

class ThermalInputController : public SacInputController
{
   Q_OBJECT

public:
   ThermalInputController(ThermalInputTab* inputTab,
                          ThermalScenario& casaScenario,
                          ScriptRunController& scriptRunController,
                          QObject* parent);

private slots:
   void slotImportTargetsClicked();
   void slotPushButtonSelectProject3dClicked() final;
   void slotUpdateTabGUI(int tabID) final;
   void slotMinTCHPChanged(QString text);
   void slotMaxTCHPChanged(QString text);

private:
   ThermalScenario& scenario() final;
   ThermalScenario& scenario() const final;

   ThermalInputTab* inputTab() final;
   const ThermalInputTab* inputTab() const final;

   void refreshGUI() final;
   std::unique_ptr<SACScript> optimizationScript(const QString& baseDirectory,
                                                 bool doOptimization = true) final;
   void readOptimizedResults() final;

   ThermalScenario& m_scenario;
   ThermalInputTab* m_inputTab;
};

} //namespace thermal

} //namespace sac

} //namespace casaWizard


