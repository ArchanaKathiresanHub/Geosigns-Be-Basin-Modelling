//
// Copyright (C) 2014-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineConductivity.h"

#include <cmath>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "IBSinterpolator2d.h"
#include "NumericFunctions.h"

 
GeoPhysics::Brine::Conductivity::Conductivity( const double salinity ) :
   GeoPhysics::Brine::Phases(salinity)
{}

void GeoPhysics::Brine::Conductivity::setTable (const ibs::Interpolator2d& thermalConductivitytbl)
{
   int tempArraySize = s_tempArraySize;
   int presArraySize = s_presArraySize;
   int thCondArraySize = s_thCondArraySize;

   thCondTable = const_cast<ibs::Interpolator2d*> (&thermalConductivitytbl);

   std::vector<ibs::XYF> vectorOfValues = vector<ibs::XYF> (thCondTable->getVectorOfValues ());
   std::vector<ibs::XYF>::iterator pointIterator;
   int i = 0;
   for (pointIterator = vectorOfValues.begin (); pointIterator != vectorOfValues.end (); ++pointIterator, ++i)
   {
      // Get value of thermal conductivity
      m_thCondArray.push_back ((*pointIterator).getF ());

      // Get value for temperature
      if (i < tempArraySize)
      {
         m_tempArray.push_back ((*pointIterator).getX ());
      }

      // Get value for pressure
      if (i % tempArraySize == 0)
      {
         m_presArray.push_back ((*pointIterator).getY ());
      }
   }
}

double GeoPhysics::Brine::Conductivity::chooseRegion( const double temperature,
                                                      const double pressure,
                                                      const double higherTemperature,
                                                      const double lowerTemperature ) const
{
   double newPressure    = pressure;
   double newTemperature = temperature;

   double highTemp = higherTemperature;
   double lowTemp  = lowerTemperature;

   if ( newPressure > PressureMaxForConductivity )
   {
      newPressure = PressureMaxForConductivity;

      highTemp = findT2( newPressure );
      lowTemp = findT1( highTemp );
   }

   if ( newTemperature > TemperatureMaxForConductivity )
   {
      newTemperature = TemperatureMaxForConductivity;
   }

   if ( newTemperature <= lowTemp )
   {
      return aqueousTable( newTemperature, newPressure );
   }
   else if ( newTemperature >= highTemp )
   {
      return vapourTable( newTemperature, newPressure );
   }
   else
   {
      return transitionRegion( newTemperature, newPressure, highTemp, lowTemp );
   }

}

// Interpolation between last aqueous value (at T1) and first vapour value (at T2).
double GeoPhysics::Brine::Conductivity::transitionRegion( const double temperature, const double pressure,
                                                          const double higherTemperature, const double lowerTemperature) const
{
   double aqueous = aqueousTable( lowerTemperature, pressure );
   double vapour  = vapourTable( higherTemperature, pressure );
   return ( aqueous + ( temperature - lowerTemperature ) * ( vapour - aqueous ) / ( higherTemperature - lowerTemperature ) );
}

// 2D Interpolation in T-P space to find thermal conductivity.
double GeoPhysics::Brine::Conductivity::interpolate2d( const double temperature, const double pressure ) const
{
   int i = 1;
   while ( temperature > m_tempArray[i] )
   {
      ++i;
   }

   if ( i >= s_tempArraySize )
   {
      i = s_tempArraySize - 1;
   }

   int j = 1;
   while ( pressure > m_presArray[j]  )
   {
      ++j;
   }

   if ( j >= s_presArraySize )
   {
      j = s_presArraySize - 1;
   }

   int k = s_tempArraySize * j + i;

   double thCondA = ( m_thCondArray[k-s_tempArraySize] * ( temperature - m_tempArray[i-1] ) + m_thCondArray[k-s_tempArraySize-1] * ( m_tempArray[i] - temperature ) ) / 
                    ( m_tempArray[i] - m_tempArray[i-1] );

   double thCondB = ( m_thCondArray[k] * ( temperature - m_tempArray[i-1] ) + m_thCondArray[k-1] * ( m_tempArray[i] - temperature ) ) /
                    ( m_tempArray[i] - m_tempArray[i-1] );

   double conductivity = ( thCondB * ( pressure - m_presArray[j-1] ) + thCondA * ( m_presArray[j] - pressure ) ) /
                         ( m_presArray[j] - m_presArray[j-1] );

   return conductivity;
  
}
