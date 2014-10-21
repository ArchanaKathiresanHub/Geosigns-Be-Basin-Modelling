#include "DoubleExponentialPorosity.h"

#include "NumericFunctions.h"
#include "GeoPhysicalConstants.h"
#include <cmath>



namespace GeoPhysics
{
   ///Constructor
   DoubleExponentialPorosity::DoubleExponentialPorosity(double depoPorosity, double minimumMechanicalPorosity, double compactionIncrA, double compactionIncrB, double compactionDecrA, double compactionDecrB)
      : Algorithm(depoPorosity,minimumMechanicalPorosity),
        m_compactionIncrA(compactionIncrA),
        m_compactionIncrB(compactionIncrB),
        m_compactionDecrA(compactionDecrA),
        m_compactionDecrB(compactionDecrB)
   {}

   ///Double Exponential porosity function
   double DoubleExponentialPorosity::porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const
   {

      double calculatedPorosity;
      bool   loadingPhase = (ves >= maxVes);

      if (loadingPhase) {
    	  calculatedPorosity  = (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp(-m_compactionIncrA * maxVes);
          calculatedPorosity += (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp(-m_compactionIncrB * maxVes) + m_minimumMechanicalPorosity;
          
      }
      else {
          calculatedPorosity  = (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes);
          calculatedPorosity += (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes) + m_minimumMechanicalPorosity;
      }
         
      if ( includeChemicalCompaction ) {
         calculatedPorosity += chemicalCompactionTerm;
         calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumPorosity);
      }
      return calculatedPorosity;
   }

   Porosity::Model DoubleExponentialPorosity::model() const
   {
      return DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY;
   }

   ///FullCompThickness
   double DoubleExponentialPorosity::fullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const {
           
      double Solid_Thickness;

      if (m_compactionIncrA == 0.0 && m_compactionIncrB == 0.0) {
         Solid_Thickness = thickness * (1.0 - m_depoPorosity);
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
         } while (fabs(thickness - computedRealThickness) >= thickness * 0.00001 && iteration++ <= 10);

         Solid_Thickness = computedSolidThickness;
      }
      return Solid_Thickness;
      
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
   double DoubleExponentialPorosity::porosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      //
      //
      //   d Phi   d Phi   d ves       d Phi
      //   ----- = ----- x -----  =  - -----  = phiA * kA * exp(-kA*ves) + phiB * kB * exp(-kB*ves)
      //     dp    d ves     dp        d ves
      //
      //   NB: ves = lithostatic pressure - pressure 
      //
      double porosityDerivative;
      bool loadingPhase = ves >= maxVes; 

      if (loadingPhase) {
         porosityDerivative  = (m_depoPorosity - m_minimumMechanicalPorosity)/2 * m_compactionIncrA * exp(-m_compactionIncrA * maxVes);
         porosityDerivative += (m_depoPorosity - m_minimumMechanicalPorosity)/2 * m_compactionIncrB * exp(-m_compactionIncrB * maxVes);
      }
      else {
         porosityDerivative  = m_compactionDecrA * (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
         porosityDerivative += m_compactionDecrB * (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes );
      }

       return porosityDerivative;
   }


} //end of namespace GeoPhysics
