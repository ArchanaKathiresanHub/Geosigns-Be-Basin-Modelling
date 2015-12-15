#include "Porosity.h"

#include <cmath>

#include "ExponentialPorosity.h"
#include "SoilMechanicsPorosity.h"
#include "DoubleExponentialPorosity.h"
#include "GeoPhysicalConstants.h"
#include "Interface/Interface.h"

const double GeoPhysics::Porosity::SolidThicknessIterationTolerance = 0.00001;


namespace GeoPhysics
{



   Porosity
      ::Porosity(Algorithm * algorithm)
      :m_algorithm(algorithm)
   {}

   Porosity
      ::Porosity():m_algorithm()
   {}

   // Factory method: Intended to be used from SimpleLithology only

   Porosity
      Porosity::create(Model porosityModel,
      double depoPorosity,
      double minimumMechanicalPorosity,
      double compactionIncr,
      double compactionIncrA,
      double compactionIncrB,
      double compactionDecr,
      double compactionDecrA,
      double compactionDecrB,
      double soilMechanicsCompactionCoefficient)
   {
      
	  switch (porosityModel)
      {
      case DataAccess::Interface::EXPONENTIAL_POROSITY:
         return Porosity(new ExponentialPorosity(depoPorosity, minimumMechanicalPorosity, compactionIncr, compactionDecr));
      case DataAccess::Interface::SOIL_MECHANICS_POROSITY:
         return Porosity(new soilMechanicsPorosity(depoPorosity, minimumMechanicalPorosity, soilMechanicsCompactionCoefficient, depoPorosity/(1-depoPorosity)));
      case DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY:
         return Porosity(new DoubleExponentialPorosity(depoPorosity, minimumMechanicalPorosity, compactionIncrA, compactionIncrB, compactionDecrA, compactionDecrB));
      default:
         assert(false);
      }
	  
      return Porosity(0);
   }
   
   Porosity::Algorithm::Algorithm(double depoPorosity, double minimumMechanicalPorosity):
		m_depoPorosity(depoPorosity),
		m_minimumMechanicalPorosity(minimumMechanicalPorosity)
   {
	   
   }

   double Porosity::Algorithm::fullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const {

      double solidThickness;

      if ( isIncompressible ()) {
         solidThickness = thickness * (1.0 - surfacePorosity ());
      } else if (thickness == 0.0) {
         solidThickness = 0.0;
      } else {

         const bool IncludeChemicalCompaction = false;

         // If we are initialising the model for an Overpressure run
         // then we assume some overpressure. An amount that equates to VES = 0.5 * ( Pl - Ph )
         const double vesScaling = (overpressuredCompaction ? vesScaleFactor : 1.0);

         double vesTop = MaxVesValue;
         double porosityTop = porosity(vesTop, vesTop, IncludeChemicalCompaction, 0.0);
         double vesBottom;
         double porosityBottom;
         double computedSolidThickness;
         double computedRealThickness;
         int iteration = 1;

         computedSolidThickness = thickness * (1.0 - porosityTop);

         do {
            vesBottom = MaxVesValue + vesScaling * AccelerationDueToGravity * densitydiff * computedSolidThickness;
            porosityBottom = porosity(vesBottom, vesBottom, IncludeChemicalCompaction, 0.0);
            computedRealThickness = 0.5 * computedSolidThickness * (1.0 / (1.0 - porosityTop) + 1.0 / (1.0 - porosityBottom));
            computedSolidThickness = computedSolidThickness * (thickness / computedRealThickness);
         } while ( std::abs ( thickness - computedRealThickness ) >= thickness * Porosity::SolidThicknessIterationTolerance && iteration++ <= 10);

         solidThickness = computedSolidThickness;
      }

      return solidThickness;
   }

   double Porosity::Algorithm::minimumMechanicalPorosity( ) const
   {
	   return m_minimumMechanicalPorosity;
   }

   double Porosity::Algorithm::surfacePorosity() const
   {
      return m_depoPorosity;
   }
}
