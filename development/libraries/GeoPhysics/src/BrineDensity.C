//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineDensity.h"

#include <cmath>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "NumericFunctions.h"

GeoPhysics::Brine::Density::Density( const double salinity ) :
  GeoPhysics::Brine::Phases(salinity)
{}

double GeoPhysics::Brine::Density::chooseRegion( const double temperature,
                                                 const double pressure,
                                                 const double higherTemperature,
                                                 const double lowerTemperature ) const
{
   if ( temperature <= lowerTemperature )
   {
      return aqueousBatzleWang( temperature, pressure );
   }
   else if ( temperature >= higherTemperature )
   {
      return vapourIdealGas( temperature, pressure );
   }
   else
   {
      return transitionRegion( temperature, pressure, higherTemperature, lowerTemperature );
   }

}

//  Batzle-Wang formula for fluids in the aqueous (liquid) phase.
double GeoPhysics::Brine::Density::aqueousBatzleWang( const double temperature,
                                                      const double pressure ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = m_salinity;

   double dens = 1000.0 * ( s * s * ( 0.44 - 0.0033 * t )
                            + p * p * ( -3.33e-7 - 2.0e-9 * t )+
                            1.75e-9 * ( -1718.91 + t ) * ( -665.977 + t ) * ( 499.172 + t )+
                            s * ( 0.668 + 0.00008 * t + 3.0e-6 * t * t )+
                            p * ( s * ( 0.0003 - 0.000013 * t ) + s * s * ( -0.0024 + 0.000047 * t )-
                                  1.3e-11 * ( -1123.64 + t ) * ( 33476.2 - 107.125 * t + t * t )));
   return dens;

}

//  Ideal-gas formula for fluids in the vapour (gas) phase.
double GeoPhysics::Brine::Density::vapourIdealGas( const double temperature,
                                                   const double pressure ) const
{  
   const double t = temperature;
   const double p = pressure;
   const double s = m_salinity;

   return p * 1.0e2 / ( t + 273.0 ) * 1.67/1.38 * ( 18.0 * (1.0-s) + 58.44 * s );

}

// Interpolation between last aqueous value (at T1) and first vapour value (at T2).
double GeoPhysics::Brine::Density::transitionRegion( const double temperature,
                                                     const double pressure,
                                                     const double higherTemperature,
                                                     const double lowerTemperature ) const
{
   double aqueous = aqueousBatzleWang( lowerTemperature, pressure );
   double vapour  = vapourIdealGas( higherTemperature, pressure );
   return ( aqueous + ( temperature - lowerTemperature ) * ( vapour - aqueous ) / ( higherTemperature - lowerTemperature ) );

}

// drho / dT
double GeoPhysics::Brine::Density::computeDerivativeT ( const double temperature, const double pressure ) const
{
   const double temp = enforceTemperatureRanges( temperature );
   const double pres = enforcePressureRanges( pressure );

   const double higherTemperature = findT2( pres );
   const double lowerTemperature = findT1( higherTemperature );
  
   if ( temp <= 1.0/1.02 * lowerTemperature )
   {
      return derivativeTemperatureBatzleWang( temp, pres );
   }
   else if ( temp >= 1.0/0.98 * higherTemperature )
   {
      return derivativeTemperatureIdealGas( temp, pres );
   }
   else
   {
      double temp1 = 0.98 * temp;
      double temp2 = 0.99 * temp;
      double temp3 = 1.01 * temp;
      double temp4 = 1.02 * temp;

      // Formula originates in finite differences approximation of derivatives.
      // This is 4th order, good enough for Ideal Gas (linear) and B&W (cubic).
      return ( - 1./12. * ( chooseRegion( temp4, pres, higherTemperature, lowerTemperature )
             - chooseRegion( temp1, pres, higherTemperature, lowerTemperature ) ) / ( 0.01 * temp ) +
             2./3. * ( chooseRegion( temp3, pres, higherTemperature, lowerTemperature )
             - chooseRegion( temp2, pres, higherTemperature, lowerTemperature ) ) / ( 0.01 * temp ) );
   }

}

// drho / dP
double GeoPhysics::Brine::Density::computeDerivativeP ( const double temperature, const double pressure ) const
{
   const double temp = enforceTemperatureRanges( temperature );
   const double pres = enforcePressureRanges( pressure );

   const double higherTemperature = findT2( pres );
   const double lowerTemperature = findT1( higherTemperature );
  
   if ( temp <= findT1( findT2( 0.98*pres ) ) )
   {
      return derivativePressureBatzleWang( temp, pres );
   }
   else if ( temp >= findT2( 1.02 * pres ) )
   {
      return derivativePressureIdealGas( temp, pres );
   }
   else
   { 
      double pres1 = 0.98 * pres;
      double pres2 = 0.99 * pres;
      double pres3 = 1.01 * pres;
      double pres4 = 1.02 * pres;

      return ( - 1./12. * ( phaseChange( temp, pres4 ) - phaseChange( temp, pres1 ) ) / ( 0.01 * pres ) +
             2./3. * ( phaseChange( temp, pres3 ) - phaseChange( temp, pres2 ) ) / ( 0.01 * pres ) );
   }

}

// Analytic drho / dT in the vapour region.
double GeoPhysics::Brine::Density::derivativeTemperatureIdealGas( const double temperature, const double pressure ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = m_salinity;

   return - 1.0e2 * 1.67 / 1.38 / ( t + 273.0 ) / ( t + 273.0 ) * p * ( 18.0 * ( 1.0 - s ) + 58.44 * s );
}

// Analytic drho / dT in the aqueous region.
double GeoPhysics::Brine::Density::derivativeTemperatureBatzleWang( const double temperature, const double pressure ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = m_salinity;

   double densityDer;

   densityDer = (1.e-6) * ( -80.0 - 2.0 * p - 0.002 * p * p - 6.6 * t + 0.032 * p * t + 0.00525 * t * t - 0.000039 * p * t * t );
   densityDer = 1000.0 * ( densityDer + (1.e-6) * s * ( 80.0 - 13.0 * p - 3300.0 * s + 6.0 * t + 47.0 * p * s ));

   return densityDer;
}

// Analytic drho / dP in the vapour region.
double GeoPhysics::Brine::Density::derivativePressureIdealGas( const double temperature, const double pressure ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = m_salinity;

   return 1.0e2 * 1.67 / 1.38 / ( t + 273.0 ) * ( 18.0 * ( 1.0 - s ) + 58.44 * s );
}

// Analytic drho / dP in the aqueous region.
double GeoPhysics::Brine::Density::derivativePressureBatzleWang( const double temperature, const double pressure ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = m_salinity;

   double densityDer;

   densityDer = (1.e-6) * ( 489.0 - 0.666 * p - 2.0 * t - 0.004 * p * t + 0.016 * t * t - 0.000013 * t * t * t );
   densityDer = 1000.0 * ( densityDer + (1.e-6) * s * ( 300.0 - 2400.0 * s - 13.0 * t + 47.0 * s * t ));

   return densityDer;
}
