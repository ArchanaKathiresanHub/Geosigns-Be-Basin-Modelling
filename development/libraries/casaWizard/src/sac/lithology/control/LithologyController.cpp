//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyController.h"

#include "control/logDisplayController.h"
#include "control/wellPrepSACcontroller.h"
#include "control/LithologyInputController.h"
#include "control/LithologyResultsController.h"
#include "control/mapsController.h"
#include "control/t2zController.h"

#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyController::LithologyController() :
  SacController(),
  m_ui{},
  m_scenario{new CMBProjectReader()},
  m_wellPrepSACcontroller{new WellPrepSACcontroller{m_ui.wellPrepTab(), m_scenario, scriptRunController(), this}},
  m_inputController{new LithologyInputController{m_ui.inputTab(), m_scenario, scriptRunController(), this}},
  m_mapsController{new MapsController{m_ui.mapsTab(), m_scenario, scriptRunController(), this}},
  m_t2zController{new T2Zcontroller{m_ui.t2zTab(), m_scenario, scriptRunController(), this}},
  m_resultsController{new LithologyResultsController{m_ui.resultsTab(), m_scenario, scriptRunController(), this}}
{
  connect(this, SIGNAL(signalUpdateTabGUI(int)), m_wellPrepSACcontroller, SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), m_mapsController,        SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), m_resultsController,     SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), m_inputController,       SLOT(slotUpdateTabGUI(int)));
  connect(this, SIGNAL(signalUpdateTabGUI(int)), m_t2zController,         SLOT(slotUpdateTabGUI(int)));
  connect(m_wellPrepSACcontroller, SIGNAL(switchToTab(int)), this,         SLOT(slotSwitchToTab(int)));

  m_ui.show();
  constructWindow(new LogDisplayController(m_ui.logDisplay(), this));
}

MainWindow& LithologyController::mainWindow()
{
  return m_ui;
}

SacLithologyScenario& LithologyController::scenario()
{
  return m_scenario;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
