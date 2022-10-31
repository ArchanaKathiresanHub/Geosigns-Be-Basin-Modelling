//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/mainController.h"
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
class ThermalMapController;

class ThermalController : public MainController
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
   ThermalMapController* m_mapController;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
