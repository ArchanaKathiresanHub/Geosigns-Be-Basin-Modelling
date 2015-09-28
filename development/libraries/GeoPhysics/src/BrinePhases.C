//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrinePhases.h"

#include <cmath>
#include <assert.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "NumericFunctions.h"

/// Allowed ranges for temperature, pressure and salinity
const double GeoPhysics::BrinePhases::s_MinTemperature = 0.0;
const double GeoPhysics::BrinePhases::s_MaxTemperature = 1500.0;

const double GeoPhysics::BrinePhases::s_MinPressure    = 0.1;
const double GeoPhysics::BrinePhases::s_MaxPressure    = 200.0;

const double GeoPhysics::BrinePhases::s_MinSalinity    = 0.0;
const double GeoPhysics::BrinePhases::s_MaxSalinity    = 0.35;

// Half width of the phase-change region in degrees Celsius
const double GeoPhysics::BrinePhases::s_halfWidth      = 20.0;

/// Table for Marine water, representative of all brines at a given pressure
/// Values of the temperature at phase change are taken from PVTsim (P in MPa, T in Celsius)
const double GeoPhysics::BrinePhases::s_TabulatedBrineP[] = {GeoPhysics::BrinePhases::s_MinPressure,0.2,0.6,1.0,2.0,6.0,10.0,20.0,60.0,100.0,GeoPhysics::BrinePhases::s_MaxPressure};
const double GeoPhysics::BrinePhases::s_TabulatedBrineT[] = {105.0,126.0,165.0,186.0,220.0,287.0,328.0,392.0,395.0,400.0,492.0};

GeoPhysics::BrinePhases::BrinePhases() : m_pres (s_TabulatedBrineP, s_TabulatedBrineP + s_TabulatedTransitions)
{
   for ( int j = 0; j < s_TabulatedTransitions; ++j )
   {
      m_temps[j] = s_TabulatedBrineT[j] + s_halfWidth;

      if (j>0)
      {
         /// Quantities used heavily in the findT2() function. Defined here for optimization.
         interpolRatio[j-1] = ( m_temps[j] - m_temps[j-1] ) / ( m_pres[j] - m_pres[j-1] );
         interpolTerm[j-1]  = m_temps[j-1] - interpolRatio[j-1] * m_pres[j-1];
      }
   }
}

double GeoPhysics::BrinePhases::phaseChange( const double temperature, const double pressure, const double salinity ) const
{
   double temp, pres, sal;
   double higherTemperature, lowerTemperature;

   enforceRanges( temperature, pressure, salinity, temp, pres, sal );

   higherTemperature = findT2( pres );
   lowerTemperature  = findT1( higherTemperature );

   return chooseRegion( temp, pres, sal, higherTemperature, lowerTemperature );
}

double GeoPhysics::BrinePhases::findT2( const double pressure ) const
{
   double higherTemperature;

   // If very close to the first or last value of the table then just use those.
   if ( std::abs( pressure -  m_pres[0] ) < 1.0e-3 * m_pres[0] )
   {
      higherTemperature = m_temps[0];
   }
   else if ( std::abs( pressure - m_pres[s_TabulatedTransitions-1] ) < 1.0e-3 * m_pres[s_TabulatedTransitions-1] )
   {
      higherTemperature = m_temps[s_TabulatedTransitions-1];
   }
   else
   {
      std::vector<double>::const_iterator it = std::upper_bound( m_pres.begin()+1, m_pres.end(), pressure );
      
      size_t j = it - m_pres.begin();

      higherTemperature = interpolRatio[j-1] * pressure + interpolTerm[j-1];
   }

   return higherTemperature;
}

double GeoPhysics::BrinePhases::findT1 (const double higherTemperature) const
{
   return higherTemperature - 2.0 * s_halfWidth;
}

