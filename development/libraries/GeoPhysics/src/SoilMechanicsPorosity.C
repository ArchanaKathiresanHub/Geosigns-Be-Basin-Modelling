#include "SoilMechanicsPorosity.h"
#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace DataAccess;

namespace GeoPhysics
{

   ///Parameters from constructor
   soilMechanicsPorosity::soilMechanicsPorosity(double depoPorosity, double minimumMechanicalPorosity, double soilMechanicsCompactionCoefficient, double depositionVoidRatio)
      : m_depoporosity(depoPorosity)
      , m_minimumMechanicalPorosity(minimumMechanicalPorosity)
      , m_soilMechanicsCompactionCoefficient(soilMechanicsCompactionCoefficient)
      , m_depositionVoidRatio(depositionVoidRatio)
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

         calculatedPorosity = voidRatio / (1.0 + voidRatio);
      }
      else {

         voidRatio = Epsilon100 - m_soilMechanicsCompactionCoefficient * log(NumericFunctions::Maximum(ves0, maxVes) / ves0);
         phiMaxVes = voidRatio / (1.0 + voidRatio);
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
      calculatedPorosity = NumericFunctions::Minimum(calculatedPorosity, m_depoporosity);

      if (includeChemicalCompaction) {
         calculatedPorosity = calculatedPorosity + chemicalCompactionTerm;
         calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumPorosity);
      }
      return calculatedPorosity;
   };


   double soilMechanicsPorosity::surfacePorosity() const
   {
      return m_depoporosity;
   }

   Porosity::Model soilMechanicsPorosity::model() const
   {
      return DataAccess::Interface::SOIL_MECHANICS_POROSITY;
   }

   ///compute FullCompThickness
   double soilMechanicsPorosity::FullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const {

      double Solid_Thickness;

      if (m_soilMechanicsCompactionCoefficient == 0.0) {
         Solid_Thickness = thickness * (1.0 - m_depoporosity);
      }
      else if (thickness == 0.0) {
         Solid_Thickness = 0.0;
      }
      else {

         const bool Loading_Phase = true;
         const bool Include_Chemical_Compaction = false;

         // If we are initialising the model for an Overpressure run
         // then we assume some overpressure. An amount that equates to VES = 0.5 * ( Pl - Ph )
         const double VES_Scaling = (overpressuredCompaction ? vesScaleFactor : 1.0);

         double vesTop = MaxVesValue;
         double porosityTop = porosity(vesTop, vesTop, Include_Chemical_Compaction, 0.0);
         double vesBottom;
         double porosityBottom;
         double computedSolidThickness;
         double computedRealThickness;
         int iteration = 1;

         computedSolidThickness = thickness * (1.0 - porosityTop);

         do {
            vesBottom = MaxVesValue + VES_Scaling * AccelerationDueToGravity * densitydiff * computedSolidThickness;
            porosityBottom = porosity(vesBottom, vesBottom, Include_Chemical_Compaction, 0.0);
            computedRealThickness = 0.5 * computedSolidThickness * (1.0 / (1.0 - porosityTop) + 1.0 / (1.0 - porosityBottom));
            computedSolidThickness = computedSolidThickness * (thickness / computedRealThickness);

#if 0
            cout << " values " << iteration << " : "
               << setw(12) << thickness << "  "
               << setw(12) << porosityTop << "  "
               << setw(12) << porosityBottom << "  "
               << setw(12) << computedSolidThickness << "  "
               << setw(12) << computedRealThickness
               << endl;
#endif

         } while (fabs(thickness - computedRealThickness) >= thickness * 0.0005 and iteration++ <= 10);

         Solid_Thickness = computedSolidThickness;
      }
      return Solid_Thickness;
   }

   ///CompactionCoefficent
   double soilMechanicsPorosity::CompactionCoefficent() const {
      return m_soilMechanicsCompactionCoefficient;
   }

   ///DVoidRatioDVes
   double soilMechanicsPorosity::DVoidRatioDVes(const double computedVoidRatio, const double ves, const double maxVes, const bool loadingPhase) const {

      /* This is the derivative of the void-ratio w.r.t. sigma with singular point ! */
      /* If loading is TRUE then maxVes = ves */

      double D_Psi_D_Sigma;
      //
      //  d psi       d psi   d phi
      //  -------  = ------   ------
      //  d sigma     d phi   d sigma
      //

      if (ves > 0.0) {
         D_Psi_D_Sigma = -m_soilMechanicsCompactionCoefficient / ves;
      }
      else {
         D_Psi_D_Sigma = -m_soilMechanicsCompactionCoefficient / std::numeric_limits<double>::epsilon();
      }

      return D_Psi_D_Sigma;
   }

   ///PorosityDerivativeWrtVes
   double soilMechanicsPorosity::PorosityDerivativeWrtVes(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      if (ves >= maxVes) {
         double ves0 = 1.0e5; //reference effective stress

         if (ves < ves0) {
            return 0.0;
         }
         else {
            double denominator = 1.0 + porosity(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
            return -m_soilMechanicsCompactionCoefficient / ves / (denominator * denominator);
         }

      }
      else {

         double surfaceVoidRatio = m_depoporosity / (1.0 - m_depoporosity);
         double porosityAtMinVes = surfaceVoidRatio / (1.0 + surfaceVoidRatio);
         double porosityAtMaxVes = porosity(maxVes, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

         double derivative = PercentagePorosityReboundForSoilMechanics * (porosityAtMaxVes - porosityAtMinVes) / (maxVes - 1.0e5);

         return derivative;
      }
   }

   ///PorosityDerivative
   double soilMechanicsPorosity::PorosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

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

   ///DVoidRatioDP
   double soilMechanicsPorosity::DVoidRatioDP(const double ves, const double maxVes, const bool loadingPhase, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      const double Biot = 1.0;

      double psi;
      double D_Psi_D_P;

      /* This routine will return the derivative of the void-ratio (dpsi) */
      /* using a cut-off value of psi. */

      double Min_ves = 1.0e5;
      double vesUsed = NumericFunctions::Maximum(ves, Min_ves);

      double computedPorosity = porosity(vesUsed, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

      psi = computedPorosity / (1.0 - computedPorosity);

      D_Psi_D_P = -Biot *
         DVoidRatioDVes(psi, vesUsed, maxVes, loadingPhase);

      return D_Psi_D_P;

   }

   ///MinimumMechanicalPorosity
   double soilMechanicsPorosity::MinimumMechanicalPorosity( ) const
   {
      return m_minimumMechanicalPorosity;
   }

}
