//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalController.h"

#include "control/logDisplayController.h"
#include "ThermalInputController.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalController::ThermalController() :
   SacController(),
   m_ui{},
   m_inputController{new ThermalInputController{m_ui.inputTab(), this->scenario(), scriptRunController(), this}}
{
   m_ui.show();
   constructWindow(new LogDisplayController(m_ui.logDisplay(), this));
}

MainWindow& ThermalController::mainWindow()
{
  return m_ui;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
