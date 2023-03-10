#include "LithologyWindow.h"

#include "view/menuBar.h"
#include "view/wellPrepTab.h"
#include "LithologyInputTab.h"
#include "LithologyResultsTab.h"
#include "LithologyMapsTab.h"
#include "t2zTab.h"

#include "assets/LithoTabIDs.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyWindow::LithologyWindow(QWidget* parent) :
   sac::SacWindow(parent),
   m_wellPrepTab{new WellPrepTab{this}},
   m_inputTab{new LithologyInputTab{this}},
   m_resultsTab{new LithologyResultsTab{this}},
   m_mapsTab{new LithologyMapsTab{this}},
   m_t2zTab{new T2Ztab{this}}
{
   setWindowTitle("SAC Lithology Wizard");
   tabWidget()->addTab(m_wellPrepTab, "Well data preparation");
   tabWidget()->addTab(m_inputTab, "Input");
   tabWidget()->addTab(m_resultsTab, "Well log plots and results");
   tabWidget()->addTab(m_mapsTab, "Maps");
   tabWidget()->addTab(m_t2zTab, "T2Z");
   assert(tabWidget()->count() == static_cast<int>(TabID::Count));
}

WellPrepTab* LithologyWindow::wellPrepTab() const
{
   return m_wellPrepTab;
}

LithologyInputTab* LithologyWindow::inputTab() const
{
   return m_inputTab;
}

LithologyResultsTab* LithologyWindow::resultsTab() const
{
   return m_resultsTab;
}

LithologyMapsTab* LithologyWindow::mapsTab() const
{
   return m_mapsTab;
}

T2Ztab* LithologyWindow::t2zTab() const
{
   return m_t2zTab;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
