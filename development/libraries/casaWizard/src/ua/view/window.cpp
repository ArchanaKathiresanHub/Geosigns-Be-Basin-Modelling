#include "window.h"

#include "view/correlationTab.h"
#include "view/doeTab.h"
#include "view/mcmcTab.h"
#include "view/menuBarUA.h"
#include "view/qcTab.h"
#include "view/targetTab.h"
#include "view/uaTabIDs.h"

#include <cassert>

namespace casaWizard
{

namespace ua
{

Window::Window(QWidget* parent) :
  MainWindow(parent),
  doeTab_{new DoeTab(this)},
  targetTab_{new TargetTab(this)},
  qcTab_{new QCTab(this)},
  mcmcTab_{new MCMCTab(this)},
  correlationsTab_{new CorrelationTab(this)},
  menuBarUA_{new MenuBarUA(this)}
{
  setMenuBar(menuBarUA_);

  setWindowTitle("CASA Thermal Wizard");

  tabWidget()->addTab(doeTab_, "DoE");
  tabWidget()->addTab(targetTab_, "Data and prediction targets");
  tabWidget()->addTab(qcTab_, "QC");
  tabWidget()->addTab(mcmcTab_, "MCMC");
  tabWidget()->addTab(correlationsTab_, "Correlations");

  assert(tabWidget()->count() == static_cast<int>(TabID::Count));
}

DoeTab* Window::doeTab() const
{
  return doeTab_;
}

TargetTab* Window::targetTab() const
{
  return targetTab_;
}

QCTab* Window::qcTab() const
{
  return qcTab_;
}

MCMCTab* Window::mcmcTab() const
{
  return mcmcTab_;
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
