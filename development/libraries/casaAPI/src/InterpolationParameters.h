//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

namespace casa {

struct InterpolationParams
{
   int interpolationMethod;
   double idwPower;
   int smoothingMethod;
   double radius;
   int nrOfThreads = 1;
};

}
