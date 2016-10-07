//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SoilMechanicsPorosity.h"
#include "FormattingException.h"
#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

namespace GeoPhysics
{
   ///Parameters from constructor
   soilMechanicsPorosity::soilMechanicsPorosity( const double depoPorosity,
                                                 const double minimumMechanicalPorosity,
                                                 const double soilMechanicsCompactionCoefficient,
                                                 const double depositionVoidRatio ):
     Algorithm(depoPorosity,minimumMechanicalPorosity),
      m_soilMechanicsCompactionCoefficient(soilMechanicsCompactionCoefficient),
      m_depositionVoidRatio(depositionVoidRatio),
      m_percentagePorosityRebound(0.02)
   {}


   void soilMechanicsPorosity::calculate( const unsigned int n,
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

      #pragma omp simd aligned (ves, maxVes, chemicalComp, porosities)
      for( size_t i = 0; i < n; ++i)
      {
         porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   void soilMechanicsPorosity::calculate( const unsigned int n,
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

      #pragma omp simd aligned (ves, maxVes, chemicalComp, porosities, porosityDers)
      for( size_t i = 0; i < n; ++i)
      {
          porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
          porosityDers[i] = computeSingleValueDerivative( porosities[i], ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   double soilMechanicsPorosity::computeSingleValue( const double ves,
                                                     const double maxVes,
                                                     const bool includeChemicalCompaction,
                                                     const double chemicalCompactionTerm ) const
   {
      double poro = 0.0;
      const double epsilon100 = m_depositionVoidRatio;
      const bool loadingPhase = (ves >= maxVes);

      const double vesUsed = std::max(Ves0, loadingPhase ? ves : maxVes);

      double voidRatio = epsilon100 - m_soilMechanicsCompactionCoefficient * std::log(vesUsed / Ves0);

      if ( voidRatio > 0.0 )
      {
         poro = voidRatio / (1.0 + voidRatio);
      } else
      {
         poro = MinimumSoilMechanicsPorosity;
      }

      if( !loadingPhase )
      {
         voidRatio = epsilon100;
         const double phiMaxVes = poro;
         const double phiMinVes = voidRatio / (1.0 + voidRatio);

         const double slope = m_percentagePorosityRebound * (phiMaxVes - phiMinVes) / (maxVes - Ves0);

         const double intercept = ( (1.0 - m_percentagePorosityRebound) * phiMaxVes * maxVes
                                    - phiMaxVes * Ves0 + m_percentagePorosityRebound * phiMinVes * maxVes) / (maxVes - Ves0);

         poro = slope * ves + intercept;
      }

      // Force porosity to be in range 0.03 .. Surface_Porosity
      if (includeChemicalCompaction)
      {
         poro = std::max(poro, m_minimumMechanicalPorosity);
      }

      poro = std::max(poro, MinimumSoilMechanicsPorosity);
      poro = std::min(poro, m_depoPorosity);

      if (includeChemicalCompaction)
      {
         poro = poro + chemicalCompactionTerm;
         poro = std::max(poro, MinimumSoilMechanicsPorosity);
      }

      return poro;
   }


   double soilMechanicsPorosity::computeSingleValueDerivative( const double porosity,
                                                               const double ves,
                                                               const double maxVes,
                                                               const bool includeChemicalCompaction,
                                                               const double chemicalCompactionTerm ) const
   {
      //   Derivative for the loading phase
      //
      //   d Phi     d Phi   d Psi           1         -Beta                2   -Beta
      //   -----  =  ----- x -----  =  ------------- x -----  =  ( 1 - Phi )  x -----
      //   d ves     d Psi   d ves     ( 1 + Psi )^2    ves                      ves
      //
      //   Derivative for the NOT loading phase
      //
      //   d Phi              PhiMaxVes - PhiMinVes
      //   -----  =  rebPct x ---------------------
      //     dp                  maxVes - ves0
      //
      //   Psi    = void ratio
      //   Beta   = compaction coefficient
      //   rebPct = percentage porosity rebound
      //
      //   NB: in case of cutoffs to upper or lower threshold the derivative has to be 0
      double poroDer = 0.0;

      const bool loadingPhase = (ves >= maxVes);
      if( loadingPhase )
      {
         // What to do if the ves is zero, for now just take double model epsilon ( ~= O(10^-16))
         const double vesValue = std::max(ves, std::numeric_limits<double>::epsilon());
         poroDer = - std::pow(1.0 - porosity, 2) * m_soilMechanicsCompactionCoefficient / vesValue;
      }
      else
      {
         // Basically here the derivative should be M (the linear coefficient for porosity wrt ves, see above)
         const double epsilon100 = m_depositionVoidRatio;
         const double vesUsed = std::max(Ves0, loadingPhase ? ves : maxVes);
         double voidRatio = epsilon100 - m_soilMechanicsCompactionCoefficient * std::log(vesUsed / Ves0);
         const double calculatedPorosity = ( voidRatio > 0.0 ) ? voidRatio / (1.0 + voidRatio) : MinimumSoilMechanicsPorosity;

         voidRatio = epsilon100;
         const double phiMaxVes = calculatedPorosity;
         const double phiMinVes = voidRatio / (1.0 + voidRatio);

         poroDer = m_percentagePorosityRebound * (phiMaxVes - phiMinVes) / (maxVes - Ves0);  // M
      }

      // Check for cutoff to the upper or lower threshold
      // In these cases the derivative has to be 0
      if( ( porosity == MinimumSoilMechanicsPorosity ) ||
          ( porosity == m_depoPorosity ) )
      {
         poroDer = 0.0;
      }

      return poroDer;
   }

}
