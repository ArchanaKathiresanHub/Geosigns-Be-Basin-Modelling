#include "Window.h"

#include "view/mapsTabLithology.h"
#include "view/menuBar.h"
#include "view/resultsTab.h"
#include "view/inputTab.h"
#include "view/LithoTabIDs.h"
#include "view/t2zTab.h"
#include "view/wellPrepTab.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

namespace lithology {

Window::Window(QWidget* parent) :
   sac::Window(parent),
   wellPrepTab_{new WellPrepTab{this}},
   inputTab_{new InputTab{this}},
   t2zTab_ {new T2Ztab{this}},
   resultsTab_{new ResultsTab{this}},
   mapsTab_{new MapsTabLithology{this}}
{
   setWindowTitle("SAC Lithology Wizard");
   tabWidget()->addTab(wellPrepTab_, "Well data preparation");
   tabWidget()->addTab(inputTab_, "Input");
   tabWidget()->addTab(resultsTab_, "Well log plots and results");
   tabWidget()->addTab(mapsTab_, "Maps");
   tabWidget()->addTab(t2zTab_, "T2Z");
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

MapsTabLithology* Window::mapsTab() const
{
   return mapsTab_;
}

ResultsTab* Window::resultsTab() const
{
   return resultsTab_;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
