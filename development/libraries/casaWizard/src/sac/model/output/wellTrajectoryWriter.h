//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Function to write well trajectory files
#pragma once

namespace casaWizard
{

namespace sac
{

class SACScenario;

namespace wellTrajectoryWriter
{

void writeTrajectories(SACScenario& scenario);

} // namespace wellTrajectoryWriter

} // namespace sac

} // namespace casaWizard
