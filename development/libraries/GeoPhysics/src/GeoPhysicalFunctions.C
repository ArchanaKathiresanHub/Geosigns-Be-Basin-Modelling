#include "GeoPhysicalFunctions.h"

#include "GeoPhysicalConstants.h"

void GeoPhysics::computeHydrostaticPressure  ( const FluidType*  fluid,
                                               const double      seaTemperature,
                                               const double      seaBottomDepth,
                                                     double&     hydrostaticPressure ) {

  if (( fluid != 0 ) && ( seaBottomDepth > 0.0 )) {

     double H = seaBottomDepth;
     double K1;
     double K2;
     double K3;
     double K4;

    //
    //
    // Is this really necessary? I think so, if the sea bottom is fairly 
    // deep then this integration method will be more accurate. For shallower
    // sea bottoms then it probably does not make that much difference. Also,
    // the integration here uses the true fluid density, whereas the simple 
    // integration (used previously) uses a standard density (=1000 kg/m^3)
    //
    K1 = H * fluid->density ( seaTemperature, AtmosphericPressure ) * AccelerationDueToGravity * PascalsToMegaPascals;
    K2 = H * fluid->density ( seaTemperature, AtmosphericPressure + 0.5 * K1 ) * AccelerationDueToGravity * PascalsToMegaPascals;
    K3 = H * fluid->density ( seaTemperature, AtmosphericPressure + 0.5 * K2 ) * AccelerationDueToGravity * PascalsToMegaPascals;
    K4 = H * fluid->density ( seaTemperature, AtmosphericPressure + K3 ) * AccelerationDueToGravity * PascalsToMegaPascals;

    hydrostaticPressure = AtmosphericPressure + ( K1 + 2.0 * ( K2 + K3 ) + K4 ) / 6.0;

  } else if (( fluid == 0 ) && ( seaBottomDepth > 0.0 )) {
    hydrostaticPressure = AtmosphericPressure + StandardWaterDensity 
                               * AccelerationDueToGravity * seaBottomDepth * PascalsToMegaPascals;

  } else {
    hydrostaticPressure = AtmosphericPressure;
  }

}
   
