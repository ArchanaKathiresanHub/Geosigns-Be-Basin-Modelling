//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the SAC wizard
#pragma once

#include "control/mainController.h"
#include "model/SacScenario.h"

namespace casaWizard
{

namespace sac
{

class SacController : public MainController
{
   Q_OBJECT

public:
   SacController();
   virtual ~SacController() override = default;
   virtual MainWindow& mainWindow() override = 0;
};

} // namespace sac

} // namespace casaWizard
