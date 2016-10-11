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

#include "methaneSolubility.h"

#include <math.h>

namespace CBMGenerics { 

namespace methaneSolubility {

double compute(const double& salinity, const double& temperatureC, const double& pressure)
{
   if (salinity >= 0.28)
      return 0;

   double temperatureK = temperatureC + 273.15;
   double methaneSolubility = exp (-55.8111 + 7478.84 / temperatureK +
	 20.6794 * (log (temperatureK) -log(100)) + 0.753158 * log (pressure)) *
	  (16.0 / 18.0) * (1.0 - salinity / 0.28);

   return methaneSolubility;
}

} // namespace methaneSolubility

} // namespace CBMGenerics
