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
#include <stdint.h>


GeoPhysics::Brine::PhaseStateBase::PhaseStateBase( const double salinity ) :
   m_salinity( enforceSalinityRanges( salinity ) )
{}


GeoPhysics::Brine::PhaseStateBase::~PhaseStateBase()
{
   // No implementation needed
}


double GeoPhysics::Brine::PhaseStateBase::findT2( const double pressure )
{
   const int j = getTemperatureUpperBoundIdx(pressure);
   return s_interpolRatio[j-1] * pressure + s_interpolTerm[j-1];
}


int GeoPhysics::Brine::PhaseStateBase::getTemperatureUpperBoundIdx( const double pressure )
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




GeoPhysics::Brine::PhaseStateScalar::PhaseStateScalar( const double salinity ) :
   PhaseStateBase( salinity )
{}


GeoPhysics::Brine::PhaseStateScalar::~PhaseStateScalar()
{
   // No implementation needed
}


void GeoPhysics::Brine::PhaseStateScalar::set( const double temperature, const double pressure )
{
   m_inRangeTemp = enforceTemperatureRanges( temperature );
   m_inRangePres = enforcePressureRanges( pressure );

   m_highEndTransitionTemp = findT2( m_inRangePres );
   m_lowEndTransitionTemp  = findT1( m_highEndTransitionTemp );
}




GeoPhysics::Brine::PhaseStateVec::PhaseStateVec( const unsigned int n, const double salinity ) :
   PhaseStateBase( salinity ),
   m_aqueousNum( 0 ),
   m_vapourNum( 0 ),
   m_transitionNum( 0 ),
   m_aqueousIdx( nullptr ),
   m_vapourIdx( nullptr ),
   m_transitionIdx( nullptr ),
   m_size( n ),
   m_inRangePres( nullptr ),
   m_inRangeTemp( nullptr ),
   m_lowEndTransitionTemp( nullptr ),
   m_highEndTransitionTemp( nullptr )
{
   allocateArrays();
}


GeoPhysics::Brine::PhaseStateVec::~PhaseStateVec()
{
   deallocateArrays();
}


void GeoPhysics::Brine::PhaseStateVec::set( const unsigned int n,
                                            ArrayDefs::ConstReal_ptr temperature,
                                            ArrayDefs::ConstReal_ptr pressure )
{
   // Check size
   assert( n == m_size );
   // Check memory alignment
   assert( ((uintptr_t)(const void *)(temperature) % ARRAY_ALIGNMENT) == 0 );
   assert( ((uintptr_t)(const void *)(pressure) % ARRAY_ALIGNMENT) == 0 );

   enforceRanges( temperature, pressure, m_inRangeTemp, m_inRangePres );

   findT2( n, m_inRangePres, m_highEndTransitionTemp );
   findT1( n, m_highEndTransitionTemp, m_lowEndTransitionTemp );

   updatePhaseStatesVector( temperature );
}


void GeoPhysics::Brine::PhaseStateVec::findT2( const int n,
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
void GeoPhysics::Brine::PhaseStateVec::findT1( const int n,
                                               ArrayDefs::ConstReal_ptr higherTemperature,
                                               ArrayDefs::Real_ptr t1 )
{
   for(int i=0; i<n; ++i)
   {
      t1[i] = higherTemperature[i] - 2.0 * s_halfWidth;
   }
   return;
}


void GeoPhysics::Brine::PhaseStateVec::allocateArrays()
{
   m_inRangePres           = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
   m_inRangeTemp           = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
   m_lowEndTransitionTemp  = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
   m_highEndTransitionTemp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( m_size );
   m_aqueousIdx            = AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::allocate ( m_size );
   m_vapourIdx             = AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::allocate ( m_size );
   m_transitionIdx         = AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::allocate ( m_size );
}


void GeoPhysics::Brine::PhaseStateVec::deallocateArrays()
{
   if( m_inRangePres != nullptr )           AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_inRangePres );
   if( m_inRangeTemp != nullptr )           AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_inRangeTemp );
   if( m_lowEndTransitionTemp != nullptr )  AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_lowEndTransitionTemp );
   if( m_highEndTransitionTemp != nullptr ) AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( m_highEndTransitionTemp );
   if( m_aqueousIdx != nullptr )            AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::free( m_aqueousIdx );
   if( m_vapourIdx != nullptr )             AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::free( m_vapourIdx );
   if( m_transitionIdx != nullptr )         AlignedMemoryAllocator<int,    ARRAY_ALIGNMENT>::free( m_transitionIdx );
}

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
__attribute__((optimize("unroll-loops")))
#endif
void GeoPhysics::Brine::PhaseStateVec::updatePhaseStatesVector( ArrayDefs::ConstReal_ptr temperature )
{
   m_aqueousNum = 0;
   m_vapourNum = 0;
   m_transitionNum = 0;

#ifdef __INTEL_COMPILER
   #pragma unroll
#endif
   for( unsigned int i = 0; i < m_size; ++i )
   {
      if ( temperature[i] <= m_lowEndTransitionTemp[i] )
      {
         m_aqueousIdx[m_aqueousNum] = i;
         ++m_aqueousNum;
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

void GeoPhysics::Brine::PhaseStateVec::enforceRanges( ArrayDefs::ConstReal_ptr temperature,
                                                      ArrayDefs::ConstReal_ptr pressure,
                                                      ArrayDefs::Real_ptr cutTemp,
                                                      ArrayDefs::Real_ptr cutPres )
{
   for(unsigned int i=0; i<m_size; ++i)
   {
      cutTemp[i] = enforceTemperatureRanges( temperature[i] );
      cutPres[i] = enforcePressureRanges( pressure[i] );
   }
}
