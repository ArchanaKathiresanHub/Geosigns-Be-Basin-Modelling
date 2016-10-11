//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CBMGENERICS_METHANESOLUBILITY_H_
#define _CBMGENERICS_METHANESOLUBILITY_H_

#include "DllExport.h"

namespace CBMGenerics { 

namespace methaneSolubility {

double compute(const double& salinity, const double& temperatureC, const double& pressure);

} // namespace methaneSolubility

} // namespace CBMGenerics

#endif
