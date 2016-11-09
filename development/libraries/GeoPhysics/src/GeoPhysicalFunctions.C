//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "GeoPhysicalFunctions.h"

#include "GeoPhysicalConstants.h"

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AtmosphericPressureMpa;
using Utilities::Physics::AccelerationDueToGravity;
using Utilities::Physics::StandardWaterDensity;
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;




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
    K1 = H * fluid->density ( seaTemperature, AtmosphericPressureMpa ) * AccelerationDueToGravity * PaToMegaPa;
    K2 = H * fluid->density ( seaTemperature, AtmosphericPressureMpa + 0.5 * K1 ) * AccelerationDueToGravity * PaToMegaPa;
    K3 = H * fluid->density ( seaTemperature, AtmosphericPressureMpa + 0.5 * K2 ) * AccelerationDueToGravity * PaToMegaPa;
    K4 = H * fluid->density ( seaTemperature, AtmosphericPressureMpa + K3 ) * AccelerationDueToGravity * PaToMegaPa;

    hydrostaticPressure = AtmosphericPressureMpa + ( K1 + 2.0 * ( K2 + K3 ) + K4 ) / 6.0;

  } else if (( fluid == 0 ) && ( seaBottomDepth > 0.0 )) {
    hydrostaticPressure = AtmosphericPressureMpa + StandardWaterDensity 
                               * AccelerationDueToGravity * seaBottomDepth * PaToMegaPa;

  } else {
    hydrostaticPressure = AtmosphericPressureMpa;
  }

}
 
void GeoPhysics::computeHydrostaticPressureSimpleDensity  ( const FluidType*  fluid,
                                                            const double  fluidDensity,
                                                            const double  seaTemperature,
                                                            const double  seaBottomDepth,
                                                            double&     hydrostaticPressure ) {


  if (( fluid != 0 ) && ( seaBottomDepth > 0.0 )) {

    //
    //
    // Is this really necessary? I think so, if the sea bottom is fairly 
    // deep then this integration method will be more accurate. For shallower
    // sea bottoms then it probably does not make that much difference. Also,
    // the integration here uses the true fluid density, whereas the simple 
    // integration (used previously) uses a standard density (=1000 kg/m^3)
    //
  
    hydrostaticPressure = AtmosphericPressureMpa +  seaBottomDepth * fluidDensity * AccelerationDueToGravity * PaToMegaPa; 

  } else if (( fluid == 0 ) && ( seaBottomDepth > 0.0 )) {
    hydrostaticPressure = AtmosphericPressureMpa + StandardWaterDensity 
                               * AccelerationDueToGravity * seaBottomDepth * PaToMegaPa;

  } else {
    hydrostaticPressure = AtmosphericPressureMpa;
  }

}
 
