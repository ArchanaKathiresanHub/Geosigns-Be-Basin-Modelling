//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineViscosity.h"

#include <cmath>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "NumericFunctions.h"

const double GeoPhysics::BrineViscosity::VapourViscosity = 2.5e-5;


double GeoPhysics::BrineViscosity::chooseRegion( const double temperature,
                                                 const double pressure,
                                                 const double salinity,
                                                 const double higherTemperature,
                                                 const double lowerTemperature ) const
{
   if ( temperature <= lowerTemperature )
   {
      return aqueousBatzleWang( temperature, salinity );
   }
   else if ( temperature >= higherTemperature )
   {
      return vapourConstant();
   }
   else
   {
      return transitionRegion( temperature, pressure, salinity, higherTemperature, lowerTemperature );
   }

}

// Batzle-Wang formula for fluids in the aqueous (liquid) phase.
double GeoPhysics::BrineViscosity::aqueousBatzleWang( const double temperature,
						      const double salinity ) const
{
   const double t = ( temperature > 0.0 ) ? temperature : 0.0;
   const double s = salinity;

   double viscosity;

   double Term1 = pow ( s, 0.8 ) - 0.17;

   double Term2 = ( 0.42 * Term1 * Term1 + 0.045 ) * pow ( t, 0.8 );

   viscosity = 0.1 + 0.333 * s + (1.65 + 91.9 * s * s * s ) * exp ( -Term2 );

   return 0.001 * viscosity;
}

// Constant viscosity in the vapour phase.
double GeoPhysics::BrineViscosity::vapourConstant() const
{
   return VapourViscosity;

}

// Interpolation between last aquous value (at T1) and first vapour value (at T2).
double GeoPhysics::BrineViscosity::transitionRegion( const double temperature, const double pressure, const double salinity,
                                                     const double higherTemperature, const double lowerTemperature) const
{
   double aqueous = aqueousBatzleWang( lowerTemperature, salinity );
   double vapour = vapourConstant();
   double viscosity = ( aqueous + ( temperature - lowerTemperature ) * ( vapour - aqueous ) / ( higherTemperature - lowerTemperature ) );

   return viscosity;

}
