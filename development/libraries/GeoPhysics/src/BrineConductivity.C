//
// Copyright (C) 2014 Shell International Exploration & Production.
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

const double GeoPhysics::BrineConductivity::PressureMaxForConductivity = 100.0;
const double GeoPhysics::BrineConductivity::TemperatureMaxForConductivity = 800.0;

const double GeoPhysics::BrineConductivity::s_tempArray[] = {0.0,25.0,50.0,75.0,100.0,125.0,150.0,200.0,250.0,300.0,350.0,375.0,400.0,425.0,450.0,475.0,500.0,550.0,600.0,650.0,700.0,750.0,800.0}; // 23 values
const double GeoPhysics::BrineConductivity::s_presArray[] = {0.1,0.5,1.0,2.5,5.0,7.5,10.0,12.5,15.0,17.5,20.0,22.5,25.0,27.5,30.0,35.0,40.0,45.0,50.0,55.0,60.0,65.0,70.0,75.0,80.0,85.0,90.0,95.0,100.0}; // 29 values

//                                                        T(C)  0    25   50   75   100  125  150  200  250  300  350  375  400  425  450  475  500  550  600  650  700  750  800      P(MPa)  
const double GeoPhysics::BrineConductivity::s_thCondArray[] = {0.56,0.61,0.64,0.66,0.02,0.03,0.03,0.03,0.04,0.04,0.05,0.05,0.05,0.06,0.06,0.06,0.07,0.07,0.08,0.09,0.09,0.10,0.11,  // 0.1
                                                               0.56,0.61,0.64,0.66,0.68,0.68,0.68,0.03,0.04,0.04,0.05,0.05,0.06,0.06,0.06,0.06,0.07,0.07,0.08,0.09,0.09,0.10,0.11,  // 0.5
                                                               0.56,0.61,0.64,0.66,0.68,0.68,0.68,0.04,0.04,0.04,0.05,0.05,0.06,0.06,0.06,0.06,0.07,0.07,0.08,0.09,0.09,0.10,0.11,  // 1.0
                                                               0.56,0.61,0.64,0.66,0.68,0.69,0.69,0.67,0.04,0.05,0.05,0.05,0.06,0.06,0.06,0.07,0.07,0.07,0.08,0.09,0.09,0.10,0.11,  // 2.5
                                                               0.56,0.61,0.64,0.67,0.68,0.69,0.69,0.67,0.62,0.05,0.06,0.06,0.06,0.06,0.06,0.07,0.07,0.08,0.08,0.09,0.10,0.10,0.11,  // 5.0
                                                               0.57,0.61,0.64,0.67,0.68,0.69,0.69,0.67,0.62,0.06,0.06,0.06,0.06,0.07,0.07,0.07,0.07,0.08,0.08,0.09,0.10,0.10,0.11,  // 7.5
                                                               0.57,0.61,0.65,0.67,0.68,0.69,0.69,0.67,0.63,0.55,0.07,0.07,0.07,0.07,0.07,0.07,0.08,0.08,0.09,0.09,0.10,0.11,0.11,  // 10.0
                                                               0.56,0.61,0.65,0.67,0.68,0.69,0.69,0.67,0.63,0.55,0.08,0.07,0.07,0.07,0.07,0.08,0.08,0.08,0.09,0.09,0.10,0.11,0.11,  // 12.5
                                                               0.57,0.62,0.65,0.67,0.69,0.69,0.69,0.67,0.63,0.56,0.10,0.08,0.08,0.08,0.08,0.08,0.08,0.09,0.09,0.10,0.10,0.11,0.12,  // 15.0
                                                               0.57,0.62,0.65,0.67,0.69,0.69,0.69,0.67,0.63,0.56,0.44,0.10,0.09,0.09,0.08,0.08,0.09,0.09,0.09,0.10,0.11,0.11,0.12,  // 17.5
                                                               0.57,0.62,0.65,0.67,0.69,0.70,0.70,0.67,0.64,0.57,0.45,0.15,0.10,0.09,0.09,0.09,0.09,0.09,0.10,0.10,0.11,0.11,0.12,  // 20.0
                                                               0.57,0.62,0.65,0.68,0.69,0.70,0.70,0.68,0.64,0.57,0.46,0.50,0.12,0.10,0.10,0.09,0.09,0.10,0.10,0.10,0.11,0.12,0.12,  // 22.5
                                                               0.58,0.62,0.65,0.68,0.69,0.70,0.70,0.68,0.64,0.58,0.47,0.39,0.16,0.12,0.11,0.10,0.10,0.10,0.10,0.11,0.11,0.12,0.12,  // 25.0
                                                               0.58,0.62,0.65,0.68,0.69,0.70,0.70,0.69,0.65,0.58,0.48,0.41,0.23,0.14,0.12,0.11,0.11,0.10,0.11,0.11,0.11,0.12,0.12,  // 27.5
                                                               0.58,0.62,0.66,0.68,0.69,0.70,0.70,0.69,0.65,0.59,0.49,0.42,0.33,0.17,0.13,0.12,0.11,0.11,0.11,0.11,0.12,0.12,0.13,  // 30.0
                                                               0.58,0.63,0.66,0.68,0.70,0.71,0.71,0.69,0.65,0.59,0.50,0.44,0.37,0.25,0.17,0.14,0.13,0.12,0.12,0.12,0.12,0.13,0.13,  // 35.0
                                                               0.58,0.63,0.66,0.68,0.70,0.71,0.71,0.69,0.66,0.60,0.52,0.46,0.40,0.32,0.23,0.17,0.15,0.13,0.13,0.13,0.13,0.13,0.14,  // 40.0
                                                               0.59,0.63,0.66,0.69,0.70,0.71,0.71,0.70,0.66,0.61,0.53,0.48,0.42,0.35,0.28,0.21,0.18,0.15,0.14,0.13,0.13,0.14,0.14,  // 45.0
                                                               0.59,0.63,0.67,0.69,0.70,0.71,0.71,0.70,0.67,0.61,0.54,0.49,0.44,0.38,0.32,0.25,0.21,0.16,0.15,0.14,0.14,0.14,0.15,  // 50.0
                                                               0.59,0.64,0.67,0.69,0.71,0.72,0.72,0.71,0.67,0.62,0.55,0.50,0.45,0.40,0.34,0.29,0.24,0.18,0.16,0.15,0.15,0.15,0.15,  // 55.0
                                                               0.60,0.64,0.67,0.69,0.71,0.72,0.72,0.71,0.68,0.63,0.56,0.52,0.47,0.42,0.37,0.31,0.26,0.20,0.17,0.16,0.16,0.16,0.16,  // 60.0
                                                               0.60,0.64,0.67,0.70,0.71,0.72,0.72,0.71,0.68,0.63,0.57,0.53,0.48,0.43,0.38,0.34,0.29,0.22,0.19,0.17,0.17,0.16,0.16,  // 65.0
                                                               0.60,0.64,0.67,0.70,0.71,0.72,0.73,0.72,0.69,0.64,0.57,0.53,0.49,0.45,0.40,0.36,0.31,0.24,0.20,0.18,0.17,0.17,0.17,  // 70.0
                                                               0.60,0.65,0.68,0.70,0.72,0.73,0.73,0.72,0.69,0.64,0.58,0.54,0.50,0.46,0.42,0.37,0.33,0.26,0.22,0.20,0.18,0.18,0.18,  // 75.0
                                                               0.61,0.65,0.68,0.70,0.72,0.73,0.73,0.72,0.69,0.65,0.59,0.55,0.51,0.47,0.43,0.39,0.35,0.28,0.23,0.21,0.19,0.19,0.18,  // 80.0
                                                               0.61,0.65,0.68,0.70,0.72,0.73,0.73,0.72,0.70,0.65,0.59,0.56,0.52,0.48,0.44,0.40,0.36,0.30,0.25,0.22,0.20,0.20,0.19,  // 85.0
                                                               0.61,0.65,0.68,0.71,0.72,0.73,0.74,0.73,0.70,0.66,0.60,0.57,0.53,0.50,0.46,0.42,0.38,0.31,0.26,0.23,0.21,0.20,0.20,  // 90.0
                                                               0.61,0.65,0.69,0.71,0.73,0.74,0.74,0.73,0.70,0.66,0.61,0.57,0.54,0.50,0.47,0.43,0.39,0.33,0.28,0.25,0.23,0.21,0.21,  // 95.0
                                                               0.62,0.66,0.69,0.71,0.73,0.74,0.74,0.73,0.71,0.67,0.61,0.58,0.55,0.51,0.48,0.44,0.40,0.34,0.29,0.26,0.24,0.22,0.21}; // 100.0
 
GeoPhysics::BrineConductivity::BrineConductivity() : m_tempArray(s_tempArray, s_tempArray + s_tempArraySize),
                                                     m_presArray(s_presArray, s_presArray + s_presArraySize),
                                                     m_thCondArray(s_thCondArray, s_thCondArray + s_thCondArraySize)
{}

double GeoPhysics::BrineConductivity::chooseRegion( const double temperature,
					            const double pressure,
                                                    const double salinity,
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

//  Use the Sengers et al. table in the aqueous (liquid) phase.
double GeoPhysics::BrineConductivity::aqueousTable( const double temperature,
                                                    const double pressure ) const
{
   double conductivity = interpolate2d( temperature, pressure );

   return conductivity;
}

//  Use the Sengers et al. table also in the vapour (gas) phase.
double GeoPhysics::BrineConductivity::vapourTable( const double temperature,
                                                   const double pressure ) const
{  
   double conductivity = interpolate2d( temperature, pressure );

   return conductivity;
}

// Interpolation between last aqueous value (at T1) and first vapour value (at T2).
double GeoPhysics::BrineConductivity::transitionRegion( const double temperature, const double pressure,
                                                        const double higherTemperature, const double lowerTemperature) const
{
   double aqueous = aqueousTable( lowerTemperature, pressure );
   double vapour  = vapourTable( higherTemperature, pressure );
   double conductivity = ( aqueous + ( temperature - lowerTemperature ) * ( vapour - aqueous ) / ( higherTemperature - lowerTemperature ) );

   return conductivity;

}

// 2D Interpolation in T-P space to find thermal conductivity.
double GeoPhysics::BrineConductivity::interpolate2d( const double temperature, const double pressure ) const
{
   int i = 1;
   while ( temperature > m_tempArray[i]  )
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
