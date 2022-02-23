#include "window.h"

#include "view/mapsTab.h"
#include "view/menuBar.h"
#include "view/resultsTab.h"
#include "view/inputTab.h"
#include "view/sacTabIDs.h"
#include "view/t2zTab.h"
#include "view/wellPrepTab.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

Window::Window(QWidget* parent) :
  MainWindow(parent),
  wellPrepTab_{new WellPrepTab{this}},
  inputTab_{new InputTab{this}},
  t2zTab_ {new T2Ztab{this}},
  resultsTab_{new ResultsTab{this}},
  mapsTab_{new MapsTab{this}},
  menuBarSAC_{new MenuBar{this}}
{
  setMenuBar(menuBarSAC_);

  setWindowTitle("SAC Wizard");

  tabWidget()->addTab(wellPrepTab_, "Well data preparation");
  tabWidget()->addTab(inputTab_, "Input");
  tabWidget()->addTab(resultsTab_, "Well log plots and results");
  tabWidget()->addTab(mapsTab_, "Maps");
  tabWidget()->addTab(t2zTab_, "T2Z");

  setStyleSheet("QToolTip{ color: #ffffff; background-color: #323232; border: 0px; }");

  assert(tabWidget()->count() == static_cast<int>(TabID::Count));
}

WellPrepTab* Window::wellPrepTab() const
{
  return wellPrepTab_;
}

InputTab* Window::inputTab() const
{
  return inputTab_;
}

T2Ztab* Window::t2zTab() const
{
  return t2zTab_;
}

MapsTab* Window::mapsTab() const
{
  return mapsTab_;
}

ResultsTab* Window::resultsTab() const
{
  return resultsTab_;
}

MenuBar* Window::menu() const
{
  return menuBarSAC_;
}

} // namespace sac

} // namespace casaWizard
