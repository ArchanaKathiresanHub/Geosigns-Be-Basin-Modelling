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
#include "BrinePhases.h"

GeoPhysics::Brine::Viscosity::Viscosity( const double salinity ) :
  m_term1( ( 0.42 * (std::pow ( salinity, 0.8 ) - 0.17) * (std::pow ( salinity, 0.8 ) - 0.17) + 0.045 ) ),
  m_term2( 0.001 * (0.1 + 0.333 * salinity) ),
  m_term3( 0.001 * (1.65 + 91.9 * salinity * salinity * salinity) )
{}


GeoPhysics::Brine::Viscosity::~Viscosity()
{
   // Intentionally unimplemented
}


double GeoPhysics::Brine::Viscosity::get( const GeoPhysics::Brine::PhaseStateScalar & phase ) const
{
   const double temperature       = phase.getTemperature();
   const double pressure          = phase.getPressure();
   const double lowerTemperature  = phase.getLowEndTransitionTemp();
   const double higherTemperature = phase.getHighEndTransitionTemp();

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


void GeoPhysics::Brine::Viscosity::get( const GeoPhysics::Brine::PhaseStateVec & phase,
                                        ArrayDefs::Real_ptr brineProp ) const
{
   const int n = phase.getVectorSize();
   const int acqueousNum   = phase.getAqueousPhasesNum();
   const int vapourNum     = phase.getVapourPhasesNum();
   const int transitionNum = phase.getTransitionPhasesNum();

   const ArrayDefs::Int_ptr acqueousIdx   = phase.getAqueousIndices();
   const ArrayDefs::Int_ptr vapourIdx     = phase.getVapourIndices();
   const ArrayDefs::Int_ptr transitionIdx = phase.getTransitionIndices();

   const ArrayDefs::Real_ptr temperature       = phase.getTemperature();
   const ArrayDefs::Real_ptr pressure          = phase.getPressure();
   const ArrayDefs::Real_ptr higherTemperature = phase.getHighEndTransitionTemp();
   const ArrayDefs::Real_ptr lowerTemperature  = phase.getLowEndTransitionTemp();

   if( acqueousNum > n/2 )
   {
      /// aqueousBatzleWang will be applied through vectorization to all the vector elements,
      /// then the other phases will be adjusted
      for( int i=0; i < n; ++i )             brineProp [i]                = aqueousBatzleWang( temperature[i] );
      for( int i=0; i < vapourNum; ++i )     brineProp [vapourIdx[i]]     = vapourConstant();
      for( int i=0; i < transitionNum; ++i ) brineProp [transitionIdx[i]] = transitionRegion( temperature[transitionIdx[i]],
                                                                                                pressure[transitionIdx[i]],
                                                                                                higherTemperature[transitionIdx[i]],
                                                                                                lowerTemperature [transitionIdx[i]] );
   }
   else if( transitionNum > n/2 )
   {
      /// transitionRegion will be applied through vectorization to all the vector elements,
      /// then the other phases will be adjusted
      for( int i=0; i < n; ++i )           brineProp [i]                = transitionRegion( temperature[i],
                                                                                              pressure[i],
                                                                                              higherTemperature[i],
                                                                                              lowerTemperature [i] );
      for( int i=0; i < acqueousNum; ++i ) brineProp [acqueousIdx[i]]   = aqueousBatzleWang( temperature[acqueousIdx[i]] );
      for( int i=0; i < vapourNum; ++i )   brineProp [vapourIdx[i]]     = vapourConstant();
   }
   else
   {
      /// All phases will be taken care of individually without making use of any vectorization
      for( int i=0; i < vapourNum; ++i )     brineProp [vapourIdx[i]]     = vapourConstant();
      for( int i=0; i < acqueousNum; ++i )   brineProp [acqueousIdx[i]]   = aqueousBatzleWang( temperature[acqueousIdx[i]] );
      for( int i=0; i < transitionNum; ++i ) brineProp [transitionIdx[i]] = transitionRegion( temperature[transitionIdx[i]],
                                                                                              pressure[transitionIdx[i]],
                                                                                              higherTemperature[transitionIdx[i]],
                                                                                              lowerTemperature [transitionIdx[i]] );
   }
}
