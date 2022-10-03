#include "Window.h"

#include "view/InputTab.h"
//#include "view/mapsTab.h"
//#include "view/resultsTab.h"

#include "view/assets/ThermalTabIDs.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

namespace thermal {

Window::Window(QWidget* parent) :
  sac::Window(parent),
  inputTab_{new InputTab{this}}
//  resultsTab_{new ResultsTab{this}},
//  mapsTab_{new MapsTab{this}},
{
  setWindowTitle("SAC Thermal Wizard");
  tabWidget()->addTab(inputTab_, "Input");
//  tabWidget()->addTab(resultsTab_, "Well log plots and results");
//  tabWidget()->addTab(mapsTab_, "Maps");
  assert(tabWidget()->count() == static_cast<int>(TabID::Count));
}

InputTab* Window::inputTab() const
{
  return inputTab_;
}

MapsTab* Window::mapsTab() const
{
  return mapsTab_;
}

ResultsTab* Window::resultsTab() const
{
  return resultsTab_;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
