#include "SacWindow.h"

#include "view/menuBar.h"
#include "view/SacInputTab.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

Window::Window(QWidget* parent) :
   MainWindow(parent),
   menuBarSAC_{new MenuBar{this}}
{
   setMenuBar(menuBarSAC_);
   setStyleSheet("QToolTip{ color: #ffffff; background-color: #323232; border: 0px; }");
}

MenuBar* Window::menu() const
{
   return menuBarSAC_;
}

} // namespace sac

} // namespace casaWizard
