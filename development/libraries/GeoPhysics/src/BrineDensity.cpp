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
#include "BrinePhases.h"
#include "ArrayDefinitions.h"
#include "AlignedMemoryAllocator.h"


GeoPhysics::Brine::Density::Density() :
   m_perturbatedPhases( GeoPhysics::Brine::PhaseStateVec( s_fdStencil, 0.0 ) )
{
   m_fdPres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate( s_fdStencil );
   m_fdTemp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate( s_fdStencil );
   m_fdDens = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate( s_fdStencil );
}


GeoPhysics::Brine::Density::~Density()
{
   if( m_fdPres != nullptr ) AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_fdPres );
   if( m_fdTemp != nullptr ) AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_fdTemp );
   if( m_fdDens != nullptr ) AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_fdDens );
}


double GeoPhysics::Brine::Density::get( const GeoPhysics::Brine::PhaseStateScalar & phase ) const
{
   const double temperature       = phase.getTemperature();
   const double pressure          = phase.getPressure();
   const double lowerTemperature  = phase.getLowEndTransitionTemp();
   const double higherTemperature = phase.getHighEndTransitionTemp();

   if ( temperature <= lowerTemperature )
   {
      return aqueousBatzleWang( temperature, pressure, phase.getSalinity() );
   }
   else if ( temperature >= higherTemperature )
   {
      return vapourIdealGas( temperature, pressure, phase.getSalinity() );
   }
   else
   {
      return transitionRegion( temperature, pressure, phase.getSalinity(), higherTemperature, lowerTemperature );
   }
}


void GeoPhysics::Brine::Density::get( const GeoPhysics::Brine::PhaseStateVec & phase,
                                      ArrayDefs::Real_ptr brineProp ) const
{
   const int n = phase.getVectorSize();
   const int acqueousNum   = phase.getAqueousPhasesNum();
   const int vapourNum     = phase.getVapourPhasesNum();
   const int transitionNum = phase.getTransitionPhasesNum();

   const double salinity = phase.getSalinity();

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
      for( int i=0; i < n; ++i )             brineProp [i]                = aqueousBatzleWang( temperature[i],
                                                                                               pressure[i],
                                                                                               salinity );
      for( int i=0; i < vapourNum; ++i )     brineProp [vapourIdx[i]]     = vapourIdealGas( temperature[vapourIdx[i]],
                                                                                            pressure[vapourIdx[i]],
                                                                                            salinity );
      for( int i=0; i < transitionNum; ++i ) brineProp [transitionIdx[i]] = transitionRegion( temperature[transitionIdx[i]],
                                                                                              pressure[transitionIdx[i]],
                                                                                              salinity,
                                                                                              higherTemperature[transitionIdx[i]],
                                                                                              lowerTemperature [transitionIdx[i]] );
   }
   else if( transitionNum > n/2 )
   {
      /// transitionRegion will be applied through vectorization to all the vector elements,
      /// then the other phases will be adjusted
      for( int i=0; i < n; ++i )           brineProp [i]              = transitionRegion( temperature[i],
                                                                                          pressure[i],
                                                                                          salinity,
                                                                                          higherTemperature[i],
                                                                                          lowerTemperature [i] );
      for( int i=0; i < acqueousNum; ++i ) brineProp [acqueousIdx[i]] = aqueousBatzleWang( temperature[acqueousIdx[i]],
                                                                                           pressure[acqueousIdx[i]],
                                                                                           salinity );
      for( int i=0; i < vapourNum; ++i )   brineProp [vapourIdx[i]]   = vapourIdealGas( temperature[vapourIdx[i]],
                                                                                        pressure[vapourIdx[i]],
                                                                                        salinity );
   }
   else
   {
      /// All phases will be taken care of individually without making use of any vectorization
      for( int i=0; i < vapourNum; ++i )     brineProp [vapourIdx[i]]     = vapourIdealGas( temperature[vapourIdx[i]],
                                                                                            pressure[vapourIdx[i]],
                                                                                            salinity );
      for( int i=0; i < acqueousNum; ++i )   brineProp [acqueousIdx[i]]   = aqueousBatzleWang( temperature[acqueousIdx[i]],
                                                                                               pressure[acqueousIdx[i]],
                                                                                               salinity );
      for( int i=0; i < transitionNum; ++i ) brineProp [transitionIdx[i]] = transitionRegion( temperature[transitionIdx[i]],
                                                                                              pressure[transitionIdx[i]],
                                                                                              salinity,
                                                                                              higherTemperature[transitionIdx[i]],
                                                                                              lowerTemperature [transitionIdx[i]] );
   }
}


// SCALAR CASE: drho / dT
double GeoPhysics::Brine::Density::computeDerivativeT( const GeoPhysics::Brine::PhaseStateScalar & phase ) const
{
   return computeDerivativeT( phase.getTemperature(),
                              phase.getPressure(),
                              phase.getSalinity(),
                              phase.getLowEndTransitionTemp(),
                              phase.getHighEndTransitionTemp() );
}


// VECTOR CASE: drho / dT
void GeoPhysics::Brine::Density::computeDerivativeT( const GeoPhysics::Brine::PhaseStateVec & phases,
                                                     ArrayDefs::Real_ptr derivative ) const
{
   const int n = phases.getVectorSize();
   const double salinity = phases.getSalinity();
   const ArrayDefs::Real_ptr temperature       = phases.getTemperature();
   const ArrayDefs::Real_ptr pressure          = phases.getPressure();
   const ArrayDefs::Real_ptr higherTemperature = phases.getHighEndTransitionTemp();
   const ArrayDefs::Real_ptr lowerTemperature  = phases.getLowEndTransitionTemp();

   for( int k=0; k < n; ++k )
   {
      derivative[k] = computeDerivativeT( temperature[k],
                                          pressure[k],
                                          salinity,
                                          lowerTemperature[k],
                                          higherTemperature[k] );
   }
}


// drho / dT
double GeoPhysics::Brine::Density::computeDerivativeT( const double temperature,
                                                       const double pressure,
                                                       const double salinity,
                                                       const double lowerTemperature,
                                                       const double higherTemperature ) const
{
   if ( temperature <= 1.0/1.02 * lowerTemperature)
   {
      return derivativeTemperatureBatzleWang( temperature, pressure, salinity );
   }
   else if ( temperature >= 1.0/0.98 * higherTemperature )
   {
      return derivativeTemperatureIdealGas( temperature, pressure, salinity );
   }
   else
   {
      for( int i = 0; i < s_fdStencil; ++i )
      {
         m_fdPres[i] = pressure;
         m_fdDens[i] = 0.0;
      }
      m_fdTemp[0] = 0.98 * temperature;
      m_fdTemp[1] = 0.99 * temperature;
      m_fdTemp[2] = 1.01 * temperature;
      m_fdTemp[3] = 1.02 * temperature;

      m_perturbatedPhases.setSalinity( salinity );
      m_perturbatedPhases.set( s_fdStencil, m_fdTemp, m_fdPres );
      get( m_perturbatedPhases, m_fdDens );

      // Formula originates in finite differences approximation of derivatives.
      // This is 4th order, good enough for Ideal Gas (linear) and B&W (cubic).
      return (- 1./12. * ( m_fdDens[3] - m_fdDens[0] ) + 2./3. * ( m_fdDens[2] - m_fdDens[1] ) ) / ( 0.01 * temperature );
   }
}


// SCALAR CASE: drho / dP
double GeoPhysics::Brine::Density::computeDerivativeP( const GeoPhysics::Brine::PhaseStateScalar & phase ) const
{
   return computeDerivativeP( phase.getTemperature(),
                              phase.getPressure(),
                              phase.getSalinity(),
                              phase.getLowEndTransitionTemp(),
                              phase.getHighEndTransitionTemp() );
}


// VECTOR CASE: drho / dP
void GeoPhysics::Brine::Density::computeDerivativeP( const GeoPhysics::Brine::PhaseStateVec & phases,
                                                     ArrayDefs::Real_ptr derivative ) const
{
   const int n = phases.getVectorSize();
   const double salinity = phases.getSalinity();
   const ArrayDefs::Real_ptr temperature       = phases.getTemperature();
   const ArrayDefs::Real_ptr pressure          = phases.getPressure();
   const ArrayDefs::Real_ptr higherTemperature = phases.getHighEndTransitionTemp();
   const ArrayDefs::Real_ptr lowerTemperature  = phases.getLowEndTransitionTemp();

   for( int k=0; k < n; ++k )
   {
      derivative[k] = computeDerivativeP( temperature[k],
                                          pressure[k],
                                          salinity,
                                          lowerTemperature[k],
                                          higherTemperature[k] );
   }
}


// drho / dP
double GeoPhysics::Brine::Density::computeDerivativeP( const double temperature,
                                                       const double pressure,
                                                       const double salinity,
                                                       const double lowerTemperature,
                                                       const double higherTemperature ) const
{
   if ( temperature <= GeoPhysics::Brine::PhaseStateScalar::findT1( GeoPhysics::Brine::PhaseStateScalar::findT2( 0.98*pressure ) ) )
   {
      return derivativePressureBatzleWang( temperature, pressure, salinity );
   }
   else if ( temperature >= GeoPhysics::Brine::PhaseStateScalar::findT2( 1.02 * pressure ) )
   {
      return derivativePressureIdealGas( temperature, pressure, salinity );
   }
   else
   {
      for( int i = 0; i < s_fdStencil; ++i )
      {
         m_fdTemp[i] = temperature;
         m_fdDens[i] = 0.0;
      }
      m_fdPres[0] = 0.98 * pressure;
      m_fdPres[1] = 0.99 * pressure;
      m_fdPres[2] = 1.01 * pressure;
      m_fdPres[3] = 1.02 * pressure;

      m_perturbatedPhases.setSalinity( salinity );
      m_perturbatedPhases.set( s_fdStencil, m_fdTemp, m_fdPres );
      get( m_perturbatedPhases, m_fdDens );

      // Formula originates in finite differences approximation of derivatives.
      // This is 4th order, good enough for Ideal Gas (linear) and B&W (cubic).
      return (- 1./12. * ( m_fdDens[3] - m_fdDens[0] ) + 2./3. * ( m_fdDens[2] - m_fdDens[1] ) ) / ( 0.01 * pressure );
   }
}
