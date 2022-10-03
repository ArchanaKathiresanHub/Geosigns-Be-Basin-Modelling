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

class SacScenario;

namespace wellTrajectoryWriter
{

void writeTrajectories(SacScenario& scenario);

} // namespace wellTrajectoryWriter

} // namespace sac

} // namespace casaWizard
