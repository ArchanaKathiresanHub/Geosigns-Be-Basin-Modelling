//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacController.h"

#include "control/SacInputController.h"
#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

SacController::SacController() :
   MainController(),
   m_scenario{new CMBProjectReader()}
{}

SacScenario& SacController::scenario()
{
   return m_scenario;
}

} // namespace sac

} // namespace casaWizard
