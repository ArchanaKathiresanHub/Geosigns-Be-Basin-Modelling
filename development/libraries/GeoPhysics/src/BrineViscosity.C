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

GeoPhysics::Brine::Viscosity::Viscosity( const double salinity ) :
  GeoPhysics::Brine::Phases(salinity),
  m_term1( ( 0.42 * (std::pow ( salinity, 0.8 ) - 0.17) * (std::pow ( salinity, 0.8 ) - 0.17) + 0.045 ) ),
  m_term2( 0.001 * (0.1 + 0.333 * salinity) ),
  m_term3( 0.001 * (1.65 + 91.9 * salinity * salinity * salinity) )
{}


GeoPhysics::Brine::Viscosity::~Viscosity()
{
   // // Intentionally unimplemented
}


double GeoPhysics::Brine::Viscosity::chooseRegion( const double temperature,
                                                   const double pressure,
                                                   const double higherTemperature,
                                                   const double lowerTemperature ) const
{
   if ( temperature <= lowerTemperature )
   {
      return aqueousBatzleWang( temperature );
   }
   else if ( temperature >= higherTemperature )
   {
      return vapourConstant();
   }
   else
   {
      return transitionRegion( temperature, pressure, higherTemperature, lowerTemperature );
   }
}


void GeoPhysics::Brine::Viscosity::chooseRegion( const int n,
                                                 ArrayDefs::ConstReal_ptr temperature,
                                                 ArrayDefs::ConstReal_ptr  pressure,
                                                 ArrayDefs::ConstReal_ptr higherTemperature,
                                                 ArrayDefs::ConstReal_ptr lowerTemperature,
                                                 ArrayDefs::Real_ptr brineProp ) const
{
   if( m_acqueousNum > n/2 )
   {
      /// aqueousBatzleWang will be applied through vectorization to all the vector elements,
      /// then the other phases will be adjusted
#ifndef _MSC_VER
      #pragma omp simd aligned (temperature, brineProp)
#endif
      for ( int i=0; i < n; ++i )               brineProp [i]                  = aqueousBatzleWang( temperature[i] );
      for ( int i=0; i < m_vapourNum; ++i )     brineProp [m_vapourIdx[i]]     = vapourConstant();
      for ( int i=0; i < m_transitionNum; ++i ) brineProp [m_transitionIdx[i]] = transitionRegion( temperature[m_transitionIdx[i]],
                                                                                                   pressure[m_transitionIdx[i]],
                                                                                                   higherTemperature[m_transitionIdx[i]],
                                                                                                   lowerTemperature [m_transitionIdx[i]] );
   }
   else if( m_transitionNum > n/2 )
   {
      /// transitionRegion will be applied through vectorization to all the vector elements,
      /// then the other phases will be adjusted
#ifndef _MSC_VER
      #pragma omp simd aligned (temperature, pressure, higherTemperature, lowerTemperature, brineProp)
#endif
      for ( int i=0; i < n; ++i )             brineProp [i]                = transitionRegion( temperature[i],
                                                                                               pressure[i],
                                                                                               higherTemperature[i],
                                                                                               lowerTemperature [i] );
      for ( int i=0; i < m_acqueousNum; ++i ) brineProp [m_acqueousIdx[i]] = aqueousBatzleWang( temperature[m_acqueousIdx[i]] );
      for ( int i=0; i < m_vapourNum; ++i )   brineProp [m_vapourIdx[i]]   = vapourConstant();
   }
   else
   {
      /// All phases will be taken care of individually without making use of any vectorization
      for ( int i=0; i < m_vapourNum; ++i )     brineProp [m_vapourIdx[i]]     = vapourConstant();
      for ( int i=0; i < m_acqueousNum; ++i )   brineProp [m_acqueousIdx[i]]   = aqueousBatzleWang( temperature[m_acqueousIdx[i]] );
      for ( int i=0; i < m_transitionNum; ++i ) brineProp [m_transitionIdx[i]] = transitionRegion( temperature[m_transitionIdx[i]],
                                                                                                   pressure[m_transitionIdx[i]],
                                                                                                   higherTemperature[m_transitionIdx[i]],
                                                                                                   lowerTemperature [m_transitionIdx[i]] );
   }
}
