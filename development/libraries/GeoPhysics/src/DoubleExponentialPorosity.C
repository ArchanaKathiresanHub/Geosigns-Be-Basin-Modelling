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

#include "NumericFunctions.h"
#include "GeoPhysicalConstants.h"
#include <cmath>

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
        m_compactionDecrB(compactionDecrB),
        m_isLegacy(isLegacy)
   {}

   ///Double Exponential porosity function
   double DoubleExponentialPorosity::calculate( const double ves,
                                                const double maxVes,
                                                const bool includeChemicalCompaction,
                                                const double chemicalCompactionTerm ) const
   {
      double calculatedPorosity;
      bool   loadingPhase = (ves >= maxVes);
      //For new rock property library feature only
		const double minimumMechanicalPorosity = NumericFunctions::Maximum(m_minimumMechanicalPorosity, MinimumPorosityNonLegacy);
      
      if (m_isLegacy) //legacy behaviour
      {
         if (loadingPhase)
         {
            calculatedPorosity = 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * exp( -m_compactionIncrA * ves );
            calculatedPorosity += 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * exp( -m_compactionIncrB * ves ) + m_minimumMechanicalPorosity;
         }
         else
         {
            calculatedPorosity = 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
            calculatedPorosity += 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes ) + m_minimumMechanicalPorosity;
         }
         if (includeChemicalCompaction)
         {
            calculatedPorosity += chemicalCompactionTerm;
            calculatedPorosity = NumericFunctions::Maximum( calculatedPorosity, MinimumPorosity );
         }
      }
      else // new rock property library behaviour
      {

         if (loadingPhase)
         {
            calculatedPorosity = 0.5 * (m_depoPorosity - minimumMechanicalPorosity) * exp( -m_compactionIncrA * ves );
            calculatedPorosity += 0.5 * (m_depoPorosity - minimumMechanicalPorosity) * exp( -m_compactionIncrB * ves ) + minimumMechanicalPorosity;
         }
         else
         {
            calculatedPorosity = 0.5 * (m_depoPorosity - minimumMechanicalPorosity) * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
            calculatedPorosity += 0.5 * (m_depoPorosity - minimumMechanicalPorosity) * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes ) + minimumMechanicalPorosity;
         }

         if (includeChemicalCompaction)
         {
            calculatedPorosity += chemicalCompactionTerm;
            calculatedPorosity = NumericFunctions::Maximum( calculatedPorosity, MinimumPorosityNonLegacy );
         }
      }
      return calculatedPorosity;
   }

   bool DoubleExponentialPorosity::isIncompressible () const {
      return m_compactionIncrA == 0.0 and m_compactionIncrB == 0.0;
   }

   Porosity::Model DoubleExponentialPorosity::model() const
   {
      return DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY;
   }

   ///CompactionCoefficientA
   double DoubleExponentialPorosity::compactionCoefficientA() const {
      return m_compactionIncrA;
   }

   ///CompactionCoefficientB
   double DoubleExponentialPorosity::compactionCoefficientB() const {
      return m_compactionIncrB;
   }

   ///PorosityDerivative
   double DoubleExponentialPorosity::calculateDerivative( const double ves,
                                                          const double maxVes,
                                                          const bool includeChemicalCompaction,
                                                          const double chemicalCompactionTerm) const
   {
      //
      //   d Phi
      //   -----  = - phiA * kA * exp(-kA*ves) - phiB * kB * exp(-kB*ves)
      //   d ves
      //
      //   with phiA = phiB = 0.5 (phi_depo - phi_min - phi_chemical)
      //
      //   If there is chemical compaction and porosity is equal to MinimumPorosity the derivative is zero
      //
      //   NB: ves = lithostatic pressure - pressure 
      //
      double porosityDerivative(0.0);
      const bool loadingPhase = (ves >= maxVes); 
      const double porosityValue = calculate(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
      //For new rock property library feature only
      const double minimumMechanicalPorosity = NumericFunctions::Maximum(m_minimumMechanicalPorosity, MinimumPorosityNonLegacy);
      const double depoPorosity = NumericFunctions::Maximum( m_depoPorosity, MinimumPorosityNonLegacy );

      // Chemical compaction equations also depends on VES, should we consider adding the chemical compaction derivative to 
      // these equations?
      double localChemicalCompactionTerm = (includeChemicalCompaction?chemicalCompactionTerm:0.0);

      if (m_isLegacy) //legacy behaviour
      {
         if (includeChemicalCompaction && (porosityValue == MinimumPorosity))
         {
            porosityDerivative = 0.0;
         }
         else if (loadingPhase)
         {
            porosityDerivative -= 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * m_compactionIncrA * exp( -m_compactionIncrA * ves );
            porosityDerivative -= 0.5 * (m_depoPorosity - m_minimumMechanicalPorosity) * m_compactionIncrB * exp( -m_compactionIncrB * ves );
         }
         else
         {
            porosityDerivative -= 0.5 * m_compactionDecrA * (m_depoPorosity - m_minimumMechanicalPorosity) * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
            porosityDerivative -= 0.5 * m_compactionDecrB * (m_depoPorosity - m_minimumMechanicalPorosity) * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes );
         }
      }
      else // new rock property library behaviour
      {
         if ((porosityValue == MinimumPorosityNonLegacy))
         {
            porosityDerivative = 0.0;
         }
         else if (loadingPhase)
         {
            porosityDerivative -= 0.5 * (depoPorosity - minimumMechanicalPorosity - localChemicalCompactionTerm) * m_compactionIncrA * exp( -m_compactionIncrA * ves );
            porosityDerivative -= 0.5 * (depoPorosity - minimumMechanicalPorosity - localChemicalCompactionTerm) * m_compactionIncrB * exp( -m_compactionIncrB * ves );
         }
         else
         {
            porosityDerivative -= 0.5 * m_compactionDecrA * (depoPorosity - minimumMechanicalPorosity - localChemicalCompactionTerm) * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
            porosityDerivative -= 0.5 * m_compactionDecrB * (depoPorosity - minimumMechanicalPorosity - localChemicalCompactionTerm) * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes );
         }
      }
       return porosityDerivative;
   }

} //end of namespace GeoPhysics
