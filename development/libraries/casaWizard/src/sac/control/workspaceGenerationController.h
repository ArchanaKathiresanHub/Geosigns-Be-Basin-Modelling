//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QString>

namespace casaWizard
{

namespace sac
{

class SACScenario;

namespace workspaceGenerationController
{

  bool generateWorkSpace(QString directory, sac::SACScenario& scenario);


} // namespace workspacegenerationcontroller

} //namespace sac

} // namespace casaWizard
