//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineVelocity.h"
#include "BrinePhases.h"
#include <cmath>

GeoPhysics::Brine::Velocity::Velocity() :
   m_highEndTransitionTempMax( GeoPhysics::Brine::PhaseStateScalar::findT2(PressureMaxForVelocity) ),
   m_lowEndTransitionTempMax( GeoPhysics::Brine::PhaseStateScalar::findT1(m_highEndTransitionTempMax) )
{}

double GeoPhysics::Brine::Velocity::get( const GeoPhysics::Brine::PhaseStateScalar & phase ) const
{
   // First a check that the pressure is smaller than 100 MPa.
   // Table from Sengers et al. only goes up to 100 MPa.
   double pressure = phase.getPressure();
   double highTemp = phase.getHighEndTransitionTemp();
   double lowTemp  = phase.getLowEndTransitionTemp();

   if ( pressure > PressureMaxForVelocity )
   {
      pressure = PressureMaxForVelocity;
      highTemp = m_highEndTransitionTempMax;
      lowTemp  = m_lowEndTransitionTempMax;
   }

   const double temperature = phase.getTemperature();

   if ( temperature <= lowTemp )
   {
      return aqueousBatzleWang( temperature, pressure, phase.getSalinity() );
   }
   else if ( temperature >= highTemp )
   {
      return vapourIdealGas( temperature, phase.getSalinity() );
   }
   else
   {
      return transitionRegion( temperature, pressure, phase.getSalinity(), highTemp, lowTemp );
   }
}

// Batzle-Wang formula for fluids in the aqueous (liquid) phase.
double GeoPhysics::Brine::Velocity::aqueousBatzleWang( const double temperature, const double pressure, const double salinity ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = salinity;

   const double t2 = t * t;
   const double t3 = t2 * t;

   const double p2 = p * p;
   const double p3 = p2 * p;

   double seisvel;

   // Use Horners method here, it may improve precision and speed.
   seisvel = 1402.85 + t * ( 4.871 - 0.04783 * t + 1.487e-4 * t2 - 2.197e-7 * t3 );
   seisvel += p * ( 1.524 + 3.437e-3 * p - 1.197e-5 * p2 );
   seisvel += p * t * ( -0.0111 + t * 2.747e-4 - t2 * 6.503e-7 + t3 * 7.987e-10 );
   seisvel += p2 * t * ( 1.739e-4 - 2.135e-6 * t - 1.455e-8 * t2 + 5.230e-11 * t3 );
   seisvel += p3 * t * ( -1.628e-6 + 1.237e-8 * t + 1.327e-10 * t2 - 4.614e-13 * t3 );

   if ( s != 0.0 )
   {
      seisvel += s * ( 1170.0 - 9.6 * t + 0.055 * t2 - 8.5e-5 * t3 + 2.6 * p - 0.0029 * p * t - 0.0476 * p2 ) +
         std::pow ( s, 1.5 ) * ( 780.0 - 10.0 * p + 0.16 * p2 ) - 820.0 * s * s;
   }
   return seisvel;
}

// Use ideal-gas law for seismic velocity in the vapour phase.
double GeoPhysics::Brine::Velocity::vapourIdealGas( const double temperature, const double salinity ) const
{
   const double t = temperature;
   const double s = salinity;

   // Formula assumes an adiabatic index of 1.333 (striclty applicable only to pure water vapour).
   return std::sqrt( 1.333 * 1.38 / 1.67 * 1.0e4 / ( 18.0 * ( 1.0 - s ) + 58.44 * s ) * ( t + 273.0 ) );
}

// Interpolation between last aqueous value (at lowerTemperature) and first vapour value (at higherTemperature).
double GeoPhysics::Brine::Velocity::transitionRegion( const double temperature,
                                                      const double pressure,
                                                      const double salinity,
                                                      const double higherTemperature,
                                                      const double lowerTemperature) const
{
   double aqueous = aqueousBatzleWang( lowerTemperature, pressure, salinity );
   double vapour = vapourIdealGas( higherTemperature, salinity );
   return ( aqueous + ( temperature - lowerTemperature ) * ( vapour - aqueous ) / ( higherTemperature - lowerTemperature ) );
}
