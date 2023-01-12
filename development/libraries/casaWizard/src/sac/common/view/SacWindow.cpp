//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacWindow.h"

#include "view/menuBar.h"
#include "view/SacInputTab.h"

#include <cassert>

namespace casaWizard
{

namespace sac
{

SacWindow::SacWindow(QWidget* parent) :
   MainWindow(parent),
   menuBarSAC_{new MenuBar{this}}
{
   setMenuBar(menuBarSAC_);
   setStyleSheet("QToolTip{ color: #ffffff; background-color: #323232; border: 0px; }");
}

MenuBar* SacWindow::menu() const
{
   return menuBarSAC_;
}

} // namespace sac

} // namespace casaWizard
