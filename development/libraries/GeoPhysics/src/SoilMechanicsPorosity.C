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
   soilMechanicsPorosity::soilMechanicsPorosity(double depoPorosity, double minimumMechanicalPorosity, double soilMechanicsCompactionCoefficient, double depositionVoidRatio):
	  Algorithm(depoPorosity,minimumMechanicalPorosity),
      m_soilMechanicsCompactionCoefficient(soilMechanicsCompactionCoefficient),
      m_depositionVoidRatio(depositionVoidRatio)
   {}

   ///soilMechanicsPorosity porosity function
   double soilMechanicsPorosity::porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      const bool loadingPhase = (ves >= maxVes);

      const double ves0 = 1.0e5; //reference effective stress

      /// Depositional void-ratio.
      const double Epsilon100 = m_depositionVoidRatio;


      double calculatedPorosity;
      double voidRatio;
      double M, C;
      double phiMaxVes;
      double phiMinVes;
      double vesUsed;

      const double PercentagePorosityRebound = 0.02; // => %age porosity regain

      if (loadingPhase) {
         vesUsed = NumericFunctions::Maximum(ves, maxVes);
         vesUsed = NumericFunctions::Maximum(vesUsed, ves0);

         voidRatio = Epsilon100 - m_soilMechanicsCompactionCoefficient * log(vesUsed / ves0);

         if ( voidRatio > 0.0 ) {
            calculatedPorosity = voidRatio / (1.0 + voidRatio);
         } else {
            calculatedPorosity = MinimumSoilMechanicsPorosity;
         } 

      } else {

         voidRatio = Epsilon100 - m_soilMechanicsCompactionCoefficient * log(NumericFunctions::Maximum(ves0, maxVes) / ves0);

         if ( voidRatio > 0.0 ) {
            phiMaxVes = voidRatio / (1.0 + voidRatio);
         } else {
            phiMaxVes = MinimumSoilMechanicsPorosity;
         } 

         voidRatio = Epsilon100;
         phiMinVes = voidRatio / (1.0 + voidRatio);

         M = PercentagePorosityRebound * (phiMaxVes - phiMinVes) / (maxVes - ves0);

         C = ((1.0 - PercentagePorosityRebound) * phiMaxVes * maxVes -
            phiMaxVes * ves0 + PercentagePorosityRebound * phiMinVes * maxVes) /
            (maxVes - ves0);

         calculatedPorosity = M * ves + C;
      }

      // Force porosity to be in range 0.03 .. Surface_Porosity

      if (includeChemicalCompaction) {
         calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, m_minimumMechanicalPorosity);
      }

      calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumSoilMechanicsPorosity);
      calculatedPorosity = NumericFunctions::Minimum(calculatedPorosity, m_depoPorosity);

      if (includeChemicalCompaction) {
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
   double soilMechanicsPorosity::porosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      //
      //
      //   d Phi   d Phi   d Psi   d ves           1          Beta
      //   ----- = ----- x ----- x -----  =  ------------- x  ----
      //     dp    d Psi   d ves     dp      ( 1 + Psi )^2     ves
      //
      //
      //
      double porosityDerivative;
      double porosityValue;
      double vesValue;

      //
      // What to do if the ves is zero, for now just take double model epsilon ( ~= O(10^-16))
      //
      vesValue = NumericFunctions::Maximum(ves, std::numeric_limits<double>::epsilon());

      porosityValue = porosity(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
      porosityDerivative = pow(1.0 - porosityValue, 2) * m_soilMechanicsCompactionCoefficient / vesValue;

      return porosityDerivative;
   }

}
