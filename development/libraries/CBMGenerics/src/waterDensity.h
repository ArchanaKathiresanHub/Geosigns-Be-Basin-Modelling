//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CBMGENERICS_WATERDENSITY_H_
#define _CBMGENERICS_WATERDENSITY_H_

namespace CBMGenerics { 

namespace waterDensity {

enum FluidDensityModel {
   Calculated,
   Constant,
   Table
};

/// @brief Compute water density depends on water salinity, temperature and pressure
/// @param type defines fluid density model
/// @param density
/// @param salinity
/// @param temperature: [C]
/// @param pressure: [MPa]
double compute( FluidDensityModel type, const double& density, const double& salinity, 
                const double& temperature, const double& pressure);

double computeFunction( const double& temperature, const double& pressure, const double& salinity );

} // namespace waterDensity

} // namespace CBMGenerics

#endif
