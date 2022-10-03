//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Function to convert the data read from the track1d output file and store it in the SAC scenario

namespace casaWizard
{

namespace sac
{

class SacScenario;
class Case3DTrajectoryReader;

namespace case3DTrajectoryConvertor
{

void convertToScenario(const Case3DTrajectoryReader& reader, SacScenario& scenario, const bool optimized = true);

}

} // namespace sac

} // namespace casaWizard
