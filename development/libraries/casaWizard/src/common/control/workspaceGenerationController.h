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

class CasaScenario;

namespace workspaceGenerationController
{

  bool generateWorkSpace(QString directory, CasaScenario& scenario);

} // namespace workspacegenerationcontroller

} // namespace casaWizard
