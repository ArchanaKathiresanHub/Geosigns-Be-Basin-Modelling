#ifndef _GEOPHYSICS__GEOPHYSICAL_FUNCTIONS_H_
#define _GEOPHYSICS__GEOPHYSICAL_FUNCTIONS_H_

#include "GeoPhysicsFluidType.h"

namespace GeoPhysics {

   void computeHydrostaticPressure  ( const FluidType*  fluid,
                                      const double      seaTemperature,
                                      const double      seaBottomDepth,
                                            double&     hydrostaticPressure );
   
 
   void computeHydrostaticPressureSimpleDensity  ( const FluidType*  fluid,
                                                   const double  fluidDensity,
                                                   const double  seaTemperature,
                                                   const double  seaBottomDepth,
                                                   double&     hydrostaticPressure );
      
}

#endif // _GEOPHYSICS__GEOPHYSICAL_FUNCTIONS_H_
