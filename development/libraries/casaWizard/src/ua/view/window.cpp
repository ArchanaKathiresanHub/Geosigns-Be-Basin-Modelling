#include "window.h"

#include "view/correlationTab.h"
#include "view/ModelInputsTab.h"
#include "view/UAResultsTab.h"
#include "view/menuBarUA.h"
#include "view/ResponseSurfacesTab.h"
#include "view/targetTab.h"
#include "view/uaTabIDs.h"

#include <cassert>

namespace casaWizard
{

namespace ua
{

Window::Window(QWidget* parent) :
  MainWindow(parent),
  m_modelInputsTab{new ModelInputsTab(this)},
  targetTab_{new TargetTab(this)},
  m_responseSurfacesTab{new ResponseSurfacesTab(this)},
  m_uaResultsTab{new UAResultsTab(this)},
  correlationsTab_{new CorrelationTab(this)},
  menuBarUA_{new MenuBarUA(this)}
{
  setMenuBar(menuBarUA_);
  setWindowTitle("Thermal UA Wizard");

  tabWidget()->addTab(m_modelInputsTab, "Model Inputs");
  tabWidget()->addTab(targetTab_, "Data and prediction targets");
  tabWidget()->addTab(m_responseSurfacesTab, "Response Surfaces");
  tabWidget()->addTab(m_uaResultsTab, "UA results");
  tabWidget()->addTab(correlationsTab_, "Correlations");

  assert(tabWidget()->count() == static_cast<int>(TabID::Count));
}

ModelInputsTab* Window::modelInputsTab() const
{
  return m_modelInputsTab;
}

TargetTab* Window::targetTab() const
{
  return targetTab_;
}

ResponseSurfacesTab* Window::responseSurfacesTab() const
{
  return m_responseSurfacesTab;
}

UAResultsTab* Window::uaResultsTab() const
{
  return m_uaResultsTab;
}

CorrelationTab* Window::correlationsTab() const
{
  return correlationsTab_;
}

MenuBar* Window::menu() const
{
  return menuBarUA_;
}

MenuBarUA* Window::menuUA() const
{
  return menuBarUA_;
}

} // namespace ua

} // namespace casaWizard
