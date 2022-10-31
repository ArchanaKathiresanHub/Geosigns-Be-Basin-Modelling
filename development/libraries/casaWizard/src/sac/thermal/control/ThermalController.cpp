//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalController.h"

#include "control/logDisplayController.h"
#include "model/input/cmbProjectReader.h"
#include "ThermalInputController.h"
#include "ThermalResultsController.h"
#include "ThermalMapController.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalController::ThermalController() :
   MainController(),
   m_ui{},
   m_scenario{new CMBProjectReader()},
   m_inputController{new ThermalInputController{m_ui.inputTab(), m_scenario, scriptRunController(), this}},
   m_resultsController{new ThermalResultsController{m_ui.resultsTab(), m_scenario, scriptRunController(), this}},
   m_mapController{new ThermalMapController{m_ui.mapsTab(), m_scenario, scriptRunController(), this}}
{
   m_ui.show();

   connect(this, SIGNAL(signalUpdateTabGUI(int)), m_inputController,       SLOT(slotUpdateTabGUI(int)));
   connect(this, SIGNAL(signalUpdateTabGUI(int)), m_resultsController,     SLOT(slotUpdateTabGUI(int)));
   connect(this, SIGNAL(signalUpdateTabGUI(int)), m_mapController,         SLOT(slotUpdateTabGUI(int)));
   constructWindow(new LogDisplayController(m_ui.logDisplay(), this));
}

MainWindow& ThermalController::mainWindow()
{
   return m_ui;
}

ThermalScenario& ThermalController::scenario()
{
   return m_scenario;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
