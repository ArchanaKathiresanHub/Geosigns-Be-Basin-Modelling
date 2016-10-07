//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ExponentialPorosity.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"


namespace GeoPhysics
{
   ///Parameters from constructor
   ExponentialPorosity::ExponentialPorosity(const double depoPorosity,
                                            const double minimumMechanicalPorosity,
                                            const double compactionIncr,
                                            const double compactionDecr,
                                            const bool   isLegacy) :
      Algorithm(depoPorosity,minimumMechanicalPorosity),
      m_compactionIncr(compactionIncr),
      m_compactionDecr(compactionDecr)
   {
      m_isLegacy = isLegacy;
   }


   ///FullCompThickness
   double ExponentialPorosity::fullCompThickness(const double maxVesValue,
                                                 const double thickness,
                                                 const double densitydiff,
                                                 const double vesScaleFactor,
                                                 const bool overpressuredCompaction) const
   {
      double solidThickness;

      if (m_compactionIncr == 0.0 || densitydiff <= 0.0 ) {
         solidThickness = thickness * (1.0 - m_depoPorosity);
      }
      else {
         const double c1 = AccelerationDueToGravity * densitydiff * m_compactionIncr;

         const double c2 = (1.0 - std::exp(-c1 * thickness)) * m_depoPorosity
                         * std::exp(-m_compactionIncr * maxVesValue);

         solidThickness = std::log(1.0 - c2) / c1 + thickness;
      }

      return solidThickness;
   }


   void ExponentialPorosity::calculate( const unsigned int n,
                                        ArrayDefs::ConstReal_ptr ves,
                                        ArrayDefs::ConstReal_ptr maxVes,
                                        const bool includeChemicalCompaction,
                                        ArrayDefs::ConstReal_ptr chemicalComp,
                                        ArrayDefs::Real_ptr porosities ) const
   {
      assert( ((uintptr_t)(const void *)(ves) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(maxVes) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(chemicalComp) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(porosities) % 32) == 0 );

      #pragma omp simd aligned (ves, maxVes, chemicalComp, porosities)
      for( size_t i = 0; i < n; ++i)
      {
         porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   void ExponentialPorosity::calculate( const unsigned int n,
                                        ArrayDefs::ConstReal_ptr ves,
                                        ArrayDefs::ConstReal_ptr maxVes,
                                        const bool includeChemicalCompaction,
                                        ArrayDefs::ConstReal_ptr chemicalComp,
                                        ArrayDefs::Real_ptr porosities,
                                        ArrayDefs::Real_ptr porosityDers ) const
   {
      assert( ((uintptr_t)(const void *)(ves) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(maxVes) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(chemicalComp) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(porosities) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(porosityDers) % 32) == 0 );

      #pragma omp simd aligned (ves, maxVes, chemicalComp, porosities, porosityDers)
      for( size_t i = 0; i < n; ++i)
      {
          porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
          porosityDers[i] = computeSingleValueDerivative( porosities[i], ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   double ExponentialPorosity::computeSingleValue( const double ves,
                                                   const double maxVes,
                                                   const bool includeChemicalCompaction,
                                                   const double chemicalCompactionTerm ) const
   {
      double poro = 0.0;

      //legacy behaviour
      if (m_isLegacy)
      {
         if (includeChemicalCompaction)
         {
            if (ves >= maxVes)
            {
               poro = (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( -m_compactionIncr * ves ) + m_minimumMechanicalPorosity;
            }
            else
            {
               poro = (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes ) + m_minimumMechanicalPorosity;
            }

            poro += chemicalCompactionTerm;
            poro = std::max( poro, MinimumPorosity );
         }
         else
         {
            if (ves >= maxVes)
            {
               poro = m_depoPorosity * std::exp( -m_compactionIncr * ves );
            }
            else
            {
               poro = m_depoPorosity * std::exp( m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes );
            }
         }
      }
      //New rock property library behaviour
      else
      {
         if( ves >= maxVes )
         {
            poro = (m_depoPorosity - m_minimumNumericalMechanicalPorosity) * std::exp(-m_compactionIncr * ves) + m_minimumNumericalMechanicalPorosity;
         }
         else
         {
            poro = (m_depoPorosity - m_minimumNumericalMechanicalPorosity) * std::exp(m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes) + m_minimumNumericalMechanicalPorosity;
         }

         if (includeChemicalCompaction)
         {
            poro += chemicalCompactionTerm;
            poro = std::max(poro, MinimumPorosityNonLegacy);
         }
      }

      return poro;
   }


   double ExponentialPorosity::computeSingleValueDerivative( const double porosity,
                                                             const double ves,
                                                             const double maxVes,
                                                             const bool includeChemicalCompaction,
                                                             const double chemicalCompactionTerm ) const
   {
      //  If there is NO chemical compaction
      //
      //  d Phi
      //  ----- = - cc * Phi             //Legacy
      //  d ves
      //                or
      //  d Phi
      //  ----- = - cc * (Phi -PhiMin)   //New rock property
      //  d ves
      //
      //  else if Phi is greater than MinimumPorosity
      //
      //  d Phi
      //  ----- = - cc * ( Phi - PhiMin - chemicalCompactionTerm )
      //  d ves
      //
      //  otherwise the derivative is zero.
      double poroDer = 0.0;

      //legacy behaviour
      if (m_isLegacy)
      {
         if (includeChemicalCompaction)
         {
            if (porosity == MinimumPorosity)
            {
               poroDer = 0.0;
            }
            else if (ves >= maxVes)
            {
               poroDer = -m_compactionIncr * (porosity - m_minimumMechanicalPorosity - chemicalCompactionTerm);
            }
            else
            {
               poroDer = -m_compactionDecr * (porosity - m_minimumMechanicalPorosity - chemicalCompactionTerm);
            }
         }
         else
         {
            if (ves >= maxVes)
            {
               poroDer = -m_compactionIncr * porosity;
            }
            else
            {
               poroDer = -m_compactionDecr * porosity;
            }
         }
      }
      else
      {
         // new rock property library behaviour
         if (porosity == MinimumPorosityNonLegacy)
         {
            poroDer = 0.0;
         }
         else if (ves >= maxVes)
         {
            poroDer = -m_compactionIncr * (porosity - m_minimumNumericalMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0) );
         }
         else
         {
            poroDer = -m_compactionDecr * (porosity - m_minimumNumericalMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0) );
         }
      }

      return poroDer;
   }

}
