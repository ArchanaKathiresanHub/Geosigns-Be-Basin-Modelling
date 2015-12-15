#ifndef _CBMGENERICS_WATERDENSITY_H_
#define _CBMGENERICS_WATERDENSITY_H_

#include "DllExport.h"

namespace CBMGenerics { 

namespace waterDensity {

enum CBMGENERICS_DLL_EXPORT FluidDensityModel {
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
