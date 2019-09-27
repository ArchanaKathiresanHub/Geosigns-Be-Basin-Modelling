#include "window.h"

#include "view/menuBar.h"
#include "view/resultsTab.h"
#include "view/sacTab.h"
#include "view/sacTabIDs.h"
#include "view/t2zTab.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

Window::Window(QWidget* parent) :
  MainWindow(parent),
  sacTab_{new SACtab{this}},
  //t2zTab_ {new T2Ztab{this}},
  resultsTab_{new ResultsTab{this}},
  menuBarSAC_{new MenuBar{this}}
{
  setMenuBar(menuBarSAC_);

  setWindowTitle("SAC Wizard");

  tabWidget()->addTab(sacTab_, "SAC");
  tabWidget()->addTab(resultsTab_, "Results");
  //tabWidget()->addTab(t2zTab_, "T2Z");

  assert(tabWidget()->count() == static_cast<int>(TabID::Count));
}

SACtab* Window::sacTab() const
{
  return sacTab_;
}

//T2Ztab* Window::t2zTab() const
//{
//  return t2zTab_;
//}

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
