//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalController.h"

#include "control/logDisplayController.h"
#include "ThermalInputController.h"
#include "ThermalResultsController.h"

#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalController::ThermalController() :
   SacController(),
   m_ui{},
   m_scenario{new CMBProjectReader()},
   m_inputController{new ThermalInputController{m_ui.inputTab(), m_scenario, scriptRunController(), this}},
   m_resultsController{new ThermalResultsController{m_ui.resultsTab(), m_scenario, scriptRunController(), this}}
{
   m_ui.show();

   connect(this, SIGNAL(signalUpdateTabGUI(int)), m_inputController,       SLOT(slotUpdateTabGUI(int)));
   connect(this, SIGNAL(signalUpdateTabGUI(int)), m_resultsController,     SLOT(slotUpdateTabGUI(int)));
//   connect(this, SIGNAL(signalUpdateTabGUI(int)), m_mapsController,        SLOT(slotUpdateTabGUI(int)));
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
