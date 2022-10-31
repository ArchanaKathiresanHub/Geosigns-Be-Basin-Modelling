//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalWindow.h"

#include "ThermalInputTab.h"
#include "ThermalResultsTab.h"
#include "ThermalMapTab.h"

#include "view/assets/ThermalTabIDs.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

namespace thermal {

ThermalWindow::ThermalWindow(QWidget* parent) :
  sac::Window(parent),
  m_inputTab{new ThermalInputTab{this}},
  m_resultsTab{new ThermalResultsTab{this}},
  m_mapsTab{new ThermalMapTab{this}}
{
  setWindowTitle("SAC Thermal Wizard");
  tabWidget()->addTab(m_inputTab, "Input");
  tabWidget()->addTab(m_resultsTab, "Well log plots and results");
  tabWidget()->addTab(m_mapsTab, "Maps");
  assert(tabWidget()->count() == static_cast<int>(TabID::Count));
}

ThermalInputTab* ThermalWindow::inputTab() const
{
  return m_inputTab;
}

ThermalResultsTab* ThermalWindow::resultsTab() const
{
  return m_resultsTab;
}

ThermalMapTab* ThermalWindow::mapsTab() const
{
  return m_mapsTab;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
