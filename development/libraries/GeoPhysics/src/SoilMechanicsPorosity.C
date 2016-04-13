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
#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"

#include <cmath>
#include <iostream>
#include <iomanip>

using namespace DataAccess;

namespace GeoPhysics
{

   ///Parameters from constructor
   soilMechanicsPorosity::soilMechanicsPorosity( const double depoPorosity,
                                                 const double minimumMechanicalPorosity,
                                                 const double soilMechanicsCompactionCoefficient,
                                                 const double depositionVoidRatio ):
	  Algorithm(depoPorosity,minimumMechanicalPorosity),
      m_soilMechanicsCompactionCoefficient(soilMechanicsCompactionCoefficient),
      m_depositionVoidRatio(depositionVoidRatio)
   {}

   ///soilMechanicsPorosity porosity function
   double soilMechanicsPorosity::calculate( const double ves,
                                            const double maxVes,
                                            const bool includeChemicalCompaction,
                                            const double chemicalCompactionTerm) const
   {
      const bool loadingPhase = (ves >= maxVes);

      /// Depositional void-ratio.
      const double Epsilon100 = m_depositionVoidRatio;

      const double vesUsed = NumericFunctions::Maximum(Ves0, loadingPhase ? ves : maxVes);
      
      double voidRatio = Epsilon100 - m_soilMechanicsCompactionCoefficient * log(vesUsed / Ves0);
      
      double calculatedPorosity;
      if ( voidRatio > 0.0 )
      {
         calculatedPorosity = voidRatio / (1.0 + voidRatio);
      } else
      {
         calculatedPorosity = MinimumSoilMechanicsPorosity;
      }

      if( !loadingPhase )
      {
         voidRatio = Epsilon100;
         const double phiMaxVes = calculatedPorosity;
         const double phiMinVes = voidRatio / (1.0 + voidRatio);

         const double PercentagePorosityRebound = 0.02; // => %age porosity regain
         const double M = PercentagePorosityRebound * (phiMaxVes - phiMinVes) / (maxVes - Ves0);

         const double C = ( (1.0 - PercentagePorosityRebound) * phiMaxVes * maxVes
                            - phiMaxVes * Ves0 + PercentagePorosityRebound * phiMinVes * maxVes) / (maxVes - Ves0);

         calculatedPorosity = M * ves + C;
      }

      // Force porosity to be in range 0.03 .. Surface_Porosity

      if (includeChemicalCompaction)
      {
         calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, m_minimumMechanicalPorosity);
      }

      calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumSoilMechanicsPorosity);
      calculatedPorosity = NumericFunctions::Minimum(calculatedPorosity, m_depoPorosity);

      if (includeChemicalCompaction)
      {
         calculatedPorosity = calculatedPorosity + chemicalCompactionTerm;
         calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumPorosity);
      }

      return calculatedPorosity;
   }
   
   bool soilMechanicsPorosity::isIncompressible () const {
      return m_soilMechanicsCompactionCoefficient == 0.0;
   }

   Porosity::Model soilMechanicsPorosity::model() const
   {
      return DataAccess::Interface::SOIL_MECHANICS_POROSITY;
   }

   ///CompactionCoefficient
   double soilMechanicsPorosity::compactionCoefficient() const {
      return m_soilMechanicsCompactionCoefficient;
   }

   ///PorosityDerivative
   double soilMechanicsPorosity::calculateDerivative( const double ves,
                                                      const double maxVes,
                                                      const bool includeChemicalCompaction,
                                                      const double chemicalCompactionTerm) const
   {
      //
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
      //

      double porosityDerivative(0.0);

      const bool   loadingPhase  = (ves >= maxVes);
      const double porosityValue = calculate(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

      if( loadingPhase )
      {
         // What to do if the ves is zero, for now just take double model epsilon ( ~= O(10^-16))
         const double vesValue = NumericFunctions::Maximum(ves, std::numeric_limits<double>::epsilon());
         porosityDerivative = - pow(1.0 - porosityValue, 2) * m_soilMechanicsCompactionCoefficient / vesValue;
      }
      else
      {
         // Basically here the derivative should be M (the linear coefficient for porosity wrt ves, see above)
         const double Epsilon100 = m_depositionVoidRatio;
         const double vesUsed = NumericFunctions::Maximum(Ves0, loadingPhase ? ves : maxVes);
         double voidRatio = Epsilon100 - m_soilMechanicsCompactionCoefficient * log(vesUsed / Ves0);
         const double calculatedPorosity = ( voidRatio > 0.0 ) ? voidRatio / (1.0 + voidRatio) : MinimumSoilMechanicsPorosity;

         voidRatio = Epsilon100;
         const double phiMaxVes = calculatedPorosity;
         const double phiMinVes = voidRatio / (1.0 + voidRatio);

         const double PercentagePorosityRebound = 0.02; // => %age porosity regain
         porosityDerivative = PercentagePorosityRebound * (phiMaxVes - phiMinVes) / (maxVes - Ves0);  // M
      }

      // Check for cutoff to the upper or lower threshold
      // In these cases the derivative has to be 0
      if( includeChemicalCompaction )
      {
         if( ( porosityValue == MinimumPorosity ) ||
             ( porosityValue == m_depoPorosity ) )
         {
            porosityDerivative = 0.0;
         }
      }
      else
      {
         if( ( porosityValue == MinimumSoilMechanicsPorosity ) ||
             ( porosityValue == m_depoPorosity ) )
         {
            porosityDerivative = 0.0;
         }
      }

      return porosityDerivative;
   }

}
