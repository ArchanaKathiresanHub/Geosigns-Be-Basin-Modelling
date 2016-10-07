//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "DoubleExponentialPorosity.h"

#include <algorithm>
#include <cmath>

#include "FormattingException.h"
#include "NumericFunctions.h"
#include "GeoPhysicalConstants.h"

namespace GeoPhysics
{
   ///Constructor
   DoubleExponentialPorosity::DoubleExponentialPorosity( const double depoPorosity,
                                                         const double minimumMechanicalPorosity,
                                                         const double compactionIncrA,
                                                         const double compactionIncrB,
                                                         const double compactionDecrA,
                                                         const double compactionDecrB,
                                                         const bool   isLegacy)
      : Algorithm(depoPorosity,minimumMechanicalPorosity),
        m_compactionIncrA(compactionIncrA),
        m_compactionIncrB(compactionIncrB),
        m_compactionDecrA(compactionDecrA),
        m_compactionDecrB(compactionDecrB)
   {
      m_isLegacy = isLegacy;
   }


   void DoubleExponentialPorosity::calculate( const unsigned int n,
                                              ArrayDefs::ConstReal_ptr ves,
                                              ArrayDefs::ConstReal_ptr maxVes,
                                              const bool includeChemicalCompaction,
                                              ArrayDefs::ConstReal_ptr chemicalComp,
                                              ArrayDefs::Real_ptr porosities ) const
   {
      if( ((uintptr_t)(const void *)(ves) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";
      if( ((uintptr_t)(const void *)(maxVes) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";
      if( ((uintptr_t)(const void *)(chemicalComp) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";
      if( ((uintptr_t)(const void *)(porosities) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";

      #pragma omp simd aligned (ves, maxVes, chemicalCompactionTerm, porosities)
      for( size_t i = 0; i < n; ++i)
      {
         porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   void DoubleExponentialPorosity::calculate( const unsigned int n,
                                              ArrayDefs::ConstReal_ptr ves,
                                              ArrayDefs::ConstReal_ptr maxVes,
                                              const bool includeChemicalCompaction,
                                              ArrayDefs::ConstReal_ptr chemicalComp,
                                              ArrayDefs::Real_ptr porosities,
                                              ArrayDefs::Real_ptr porosityDers ) const
   {
      if( ((uintptr_t)(const void *)(ves) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";
      if( ((uintptr_t)(const void *)(maxVes) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";
      if( ((uintptr_t)(const void *)(chemicalComp) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";
      if( ((uintptr_t)(const void *)(porosities) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";
      if( ((uintptr_t)(const void *)(porosityDers) % 32) != 0 ) throw formattingexception::GeneralException() << __FUNCTION__ << " unaligned memory";

      #pragma omp simd aligned (ves, maxVes, chemicalCompactionTerm, porosities, porosityDers)
      for( size_t i = 0; i < n; ++i)
      {
          porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
          porosityDers[i] = computeSingleValueDerivative( porosities[i], ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   double DoubleExponentialPorosity::computeSingleValue( const double ves,
                                                         const double maxVes,
                                                         const bool includeChemicalCompaction,
                                                         const double chemicalCompactionTerm ) const
   {
      double poro = 0.0;

      //legacy behaviour
      if (m_isLegacy)
      {
         if (ves >= maxVes)
         {
            poro = 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( -m_compactionIncrA * ves );
            poro += 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( -m_compactionIncrB * ves ) + m_minimumMechanicalPorosity;
         }
         else
         {
            poro = 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
            poro += 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes ) + m_minimumMechanicalPorosity;
         }
         if (includeChemicalCompaction)
         {
            poro += chemicalCompactionTerm;
            poro = NumericFunctions::Maximum( poro, MinimumPorosity );
         }
      }
      else
      {
         // new rock property library behaviour
         if (ves >= maxVes)
         {
            poro = 0.5 * (m_depoPorosity - m_minimumNumericalMechanicalPorosity)
                 * ( std::exp(-m_compactionIncrA * ves) + std::exp(-m_compactionIncrB * ves) ) + m_minimumNumericalMechanicalPorosity;
         }
         else
         {
            poro = 0.5 * (m_depoPorosity - m_minimumNumericalMechanicalPorosity)
                 * ( std::exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes) + std::exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes) ) + m_minimumNumericalMechanicalPorosity;
         }

         if (includeChemicalCompaction)
         {
            poro += chemicalCompactionTerm;
            poro = std::max(poro, MinimumPorosityNonLegacy);
         }
      }

      return poro;
   }


   double DoubleExponentialPorosity::computeSingleValueDerivative( const double porosity,
                                                                   const double ves,
                                                                   const double maxVes,
                                                                   const bool includeChemicalCompaction,
                                                                   const double chemicalCompactionTerm ) const
   {
      //   d Phi
      //   -----  = - phiA * kA * exp(-kA*ves) - phiB * kB * exp(-kB*ves)
      //   d ves
      //
      //   with phiA = phiB = 0.5 (phi_depo - phi_min - phi_chemical)
      //
      //   If there is chemical compaction and porosity is equal to MinimumPorosity the derivative is zero
      //
      //   NB: ves = lithostatic pressure - pressure
      double poroDer = 0.0;

      // Chemical compaction equations also depends on VES, should we consider adding the chemical compaction derivative to
      // these equations?

      //legacy behaviour
      if (m_isLegacy)
      {
         if (includeChemicalCompaction && (porosity == MinimumPorosity))
         {
            poroDer = 0.0;
         }
         else if (ves >= maxVes)
         {
            poroDer -= 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * m_compactionIncrA * std::exp( -m_compactionIncrA * ves );
            poroDer -= 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * m_compactionIncrB * std::exp( -m_compactionIncrB * ves );
         }
         else
         {
            poroDer -= 0.5 * m_compactionDecrA * (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
            poroDer -= 0.5 * m_compactionDecrB * (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes );
         }
      }
      else
      {
         // new rock property library behaviour
         const double localChemicalCompactionTerm = (includeChemicalCompaction?chemicalCompactionTerm:0.0);
         const double coeff = 0.5 * (m_minimumNumericalDepoPorosity - m_minimumNumericalMechanicalPorosity - localChemicalCompactionTerm);

         if( (porosity == MinimumPorosityNonLegacy) )
         {
            poroDer = 0.0;
         }
         else if (ves >= maxVes)
         {
            poroDer -= coeff * ( m_compactionIncrA * std::exp(-m_compactionIncrA * ves) + m_compactionIncrB * std::exp(-m_compactionIncrB * ves) );
         }
         else
         {
            poroDer -= coeff * ( m_compactionDecrA * std::exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes ) + m_compactionDecrB * std::exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes ) );
         }
      }

      return poroDer;
   }

} //end of namespace GeoPhysics
