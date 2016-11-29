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
#include "AlignedMemoryAllocator.h"

#include <cmath>
#include <cassert>
#include <algorithm>


GeoPhysics::Brine::Phases::Phases( const double salinity ) :
   m_salinity  ( salinity > s_MaxSalinity ? s_MaxSalinity : ( salinity < s_MinSalinity ? s_MinSalinity : salinity ) ),
   m_acqueousNum( 0 ),
   m_vapourNum( 0 ),
   m_transitionNum( 0 ),
   m_acqueousIdx( nullptr ),
   m_vapourIdx( nullptr ),
   m_transitionIdx( nullptr ),
   m_size( 0 ),
   m_inRangePres( nullptr ),
   m_inRangeTemp( nullptr ),
   m_lowEndTransitionTemp( nullptr ),
   m_highEndTransitionTemp( nullptr )
{
   allocateVariableArrays( m_size );
}


GeoPhysics::Brine::Phases::~Phases()
{
   deallocateVariableArrays();
}


void GeoPhysics::Brine::Phases::setVectorSize( const unsigned int size )
{
   allocateVariableArrays( size );
}


double GeoPhysics::Brine::Phases::phaseChange( const double temperature, const double pressure ) const
{
   const double temp = enforceTemperatureRanges( temperature );
   const double pres = enforcePressureRanges( pressure );

   const double higherTemperature = findT2( pres );
   const double lowerTemperature  = findT1( higherTemperature );

   return chooseRegion( temp, pres, higherTemperature, lowerTemperature );
}


void GeoPhysics::Brine::Phases::phaseChange( const unsigned int n,
                                             ArrayDefs::ConstReal_ptr temperature,
                                             ArrayDefs::ConstReal_ptr pressure,
                                             ArrayDefs::Real_ptr brineProp )
{
   // Check memory alignment
   assert( ((uintptr_t)(const void *)(temperature) % ARRAY_ALIGNMENT) == 0 );
   assert( ((uintptr_t)(const void *)(pressure) % ARRAY_ALIGNMENT) == 0 );
   assert( ((uintptr_t)(const void *)(brineProp) % ARRAY_ALIGNMENT) == 0 );

   allocateVariableArrays( n );

   enforceRanges( n, temperature, pressure, m_inRangeTemp, m_inRangePres );

   findT2( n, m_inRangePres, m_highEndTransitionTemp );
   findT1( n, m_highEndTransitionTemp, m_lowEndTransitionTemp );
   
   updatePhaseStatesVector( n, temperature );
   
   chooseRegion( n, m_inRangeTemp, m_inRangePres, m_highEndTransitionTemp, m_lowEndTransitionTemp, brineProp );
   return;
}


double GeoPhysics::Brine::Phases::findT2( const double pressure )
{
   const int j = getTemperatureUpperBoundIdx(pressure);
   return s_interpolRatio[j-1] * pressure + s_interpolTerm[j-1];
}


void GeoPhysics::Brine::Phases::findT2( const int n,
                                        ArrayDefs::ConstReal_ptr pressure,
                                        ArrayDefs::Real_ptr t2 )
{
   int idx = 0;

   // Non unit stride access here, it's not worth vectorizing
#ifdef __INTEL_COMPILER
   // The trip count after loop unrolling is too small compared to the vector length. To fix: Prevent loop unrolling
   #pragma nounroll
#endif
   for(int i=0; i<n; ++i)
   {
      idx = getTemperatureUpperBoundIdx( pressure[i] );
      t2[i] = s_interpolRatio[idx-1] * pressure[i] + s_interpolTerm[idx-1];
   }

   return;
}


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
__attribute__((optimize("unroll-loops")))
#endif
void GeoPhysics::Brine::Phases::findT1( const int n,
                                        ArrayDefs::ConstReal_ptr higherTemperature,
                                        ArrayDefs::Real_ptr t1 )
{
#ifdef __INTEL_COMPILER
   // The trip count after loop unrolling is too small compared to the vector length. To fix: Prevent loop unrolling
   #pragma omp simd aligned (higherTemperature, t1)
   #pragma nounroll
#endif
   for(int i=0; i<n; ++i)
   {
      t1[i] = higherTemperature[i] - 2.0 * s_halfWidth;
   }
   return;
}


int GeoPhysics::Brine::Phases::getTemperatureUpperBoundIdx( const double pressure )
{
   // Since the size of the lookup array is small this piece of code is much faster
   // than the STL find functions
   unsigned int j;
   if ( s_TabulatedBrineP[5] > pressure )
   {
      if ( s_TabulatedBrineP[ 3] > pressure )
      {
         // Values smaller than s_TabulatedBrineP[0] will be extrapolated using the first piecewise interpolation
         if( s_TabulatedBrineP[ 1] > pressure ) j = 1;
         else if( s_TabulatedBrineP[ 2] > pressure ) j = 2;
         else j = 3;
      }
      else
      {
         if( s_TabulatedBrineP[ 4] > pressure ) j = 4;
         else j = 5;
      }
   }
   else
   {
      if (s_TabulatedBrineP[ 8] > pressure )
      {
         if( s_TabulatedBrineP[ 6] > pressure ) j = 6;
         else if( s_TabulatedBrineP[ 7] > pressure ) j = 7;
         else j = 8;
      }
      else
      {
         if( s_TabulatedBrineP[ 9] > pressure ) j = 9;
         else if( s_TabulatedBrineP[10] > pressure ) j = 10;
         // Values greater than s_TabulatedBrineP[10] will be extrapolated using the last piecewise interpolation
         else j = 10;
      }
   }

   return j;
}


void GeoPhysics::Brine::Phases::allocateVariableArrays( const unsigned int newSize )
{
   if( m_size == newSize ) return;
   m_size = newSize;
   deallocateVariableArrays();
   if( m_size > 0 )
   {
      m_inRangePres           = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
      m_inRangeTemp           = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
      m_lowEndTransitionTemp  = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
      m_highEndTransitionTemp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
      m_acqueousIdx           = AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::allocate ( m_size );
      m_vapourIdx             = AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::allocate ( m_size );
      m_transitionIdx         = AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::allocate ( m_size );
   }
}


void GeoPhysics::Brine::Phases::deallocateVariableArrays()
{
   if( m_inRangePres != nullptr )           AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_inRangePres );
   if( m_inRangeTemp != nullptr )           AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_inRangeTemp );
   if( m_lowEndTransitionTemp != nullptr )  AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_lowEndTransitionTemp );
   if( m_highEndTransitionTemp != nullptr ) AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_highEndTransitionTemp );
   if( m_acqueousIdx != nullptr )           AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::free( m_acqueousIdx );
   if( m_vapourIdx != nullptr )             AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::free( m_vapourIdx );
   if( m_transitionIdx != nullptr )         AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::free( m_transitionIdx );
}

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
__attribute__((optimize("unroll-loops")))
#endif
void GeoPhysics::Brine::Phases::updatePhaseStatesVector( const int n, ArrayDefs::ConstReal_ptr temperature )
{
   m_acqueousNum = 0;
   m_vapourNum = 0;
   m_transitionNum = 0;

#ifdef __INTEL_COMPILER
   #pragma unroll
#endif
   for( int i = 0; i < n; ++i )
   {
      if ( temperature[i] <= m_lowEndTransitionTemp[i] )
      {
         m_acqueousIdx[m_acqueousNum] = i;
         ++m_acqueousNum;
      }
      else if ( temperature[i] >= m_highEndTransitionTemp[i] )
      {
         m_vapourIdx[m_vapourNum] = i;
         ++m_vapourNum;
      }
      else
      {
         m_transitionIdx[m_transitionNum] = i;
         ++m_transitionNum;
      }
   }
}

void GeoPhysics::Brine::Phases::enforceRanges( const int n,
                                               ArrayDefs::ConstReal_ptr temperature,
                                               ArrayDefs::ConstReal_ptr pressure,
                                               ArrayDefs::Real_ptr cutTemp,
                                               ArrayDefs::Real_ptr cutPres )
{
#ifndef _MSC_VER
   #pragma omp simd aligned (pressure, temperature, cutTemp, cutPres)
#endif
   for(int i=0; i<n; ++i)
   {
      cutTemp[i] = enforceTemperatureRanges( temperature[i] );
      cutPres[i] = enforcePressureRanges( pressure[i] );
   }
}
