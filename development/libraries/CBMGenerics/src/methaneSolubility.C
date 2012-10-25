#include "stdafx.h"

#include "methaneSolubility.h"

#include <math.h>

namespace CBMGenerics { 

namespace methaneSolubility {

double compute(const double& salinity, const double& temperatureC, const double& pressurePa)
{
   if (salinity >= 0.28)
      return 0;

   double temperatureK = temperatureC + 273.15;
   double methaneSolubility = exp (-55.8111 + 7478.84 / temperatureK +
	 20.6794 * log (temperatureK / 100) + 0.753158 * log (pressurePa * 0.000001)) *
      (16.0 / 18.0) * (1.0 - salinity / 0.28);
   return methaneSolubility;
}

} // namespace methaneSolubility

} // namespace CBMGenerics
