//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/SacInputController.h"
#include <memory>

namespace casaWizard
{

namespace sac
{

class SACScript;

namespace thermal
{

class ThermalInputTab;

class ThermalInputController : public SacInputController
{
   Q_OBJECT

public:
   ThermalInputController(ThermalInputTab* inputTab,
                          SacScenario& casaScenario,
                          ScriptRunController& scriptRunController,
                          QObject* parent);

private slots:
   void slotImportTargetsClicked();
   void slotPushButtonSelectProject3dClicked() final;
   void slotUpdateTabGUI(int tabID) final;
   void slotMinHCPChanged(QString text);
   void slotMaxHCPChanged(QString text);

private:
   void refreshGUI() final;
   SacScenario& scenario() override;
   SacScenario& scenario() const override;
   std::unique_ptr<SACScript> optimizationScript(const QString& baseDirectory,
                                                 bool doOptimization = true) final;
   void readOptimizedResults() final;

   SacScenario& m_scenario;
   double m_minHCP;
   double m_maxHCP;
};

} //namespace thermal

} //namespace sac

} //namespace casaWizard


