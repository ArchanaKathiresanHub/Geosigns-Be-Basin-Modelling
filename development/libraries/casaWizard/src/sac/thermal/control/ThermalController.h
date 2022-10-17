//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once
#include "control/SacController.h"
#include "model/ThermalScenario.h"
#include "view/ThermalWindow.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalInputController;
class ThermalResultsController;

class ThermalController : public SacController
{
public:
   ThermalController();
   virtual ~ThermalController() override = default;
   MainWindow& mainWindow() final;
   ThermalScenario& scenario() final;

private:
   ThermalWindow m_ui;
   ThermalScenario m_scenario;
   ThermalInputController* m_inputController;
   ThermalResultsController* m_resultsController;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
