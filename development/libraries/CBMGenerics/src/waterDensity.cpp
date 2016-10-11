//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "stdafx.h"

#include "waterDensity.h"

#include <assert.h>

namespace CBMGenerics {

namespace waterDensity {

double compute(FluidDensityModel type, const double& density, const double& salinity,
   const double& Temperature, const double& pressure)
{
   assert(type != Table);
   return type == Calculated ?
      computeFunction(Temperature, pressure, salinity) :
      density;
}

double computeFunction(const double& t, const double& p,
   const double& s)
{
   double result;

   result = 1000*(s*s*(0.44 - 0.0033*t) + p*p*((-3.33e-7) - (2.0e-9)*t)+
		(1.75e-9)*(-1718.91 + t)*(-665.977 + t)*(499.172 + t)+
		s*(0.668 + 0.00008*t + (3.0e-6)*t*t)+
		p*(s*(0.0003 - 0.000013*t) + s*s*(-0.0024 + 0.000047*t)-
		   (1.3e-11)*(-1123.64 + t)*(33476.2 - 107.125*t + t*t)));
   return result;
}

// FIXME: What to do with the third possibility the table (which presently 
// you can't select in the UI).
 
} // namespace waterDensity

} // namespace DistributedDataAccess::Implementation
