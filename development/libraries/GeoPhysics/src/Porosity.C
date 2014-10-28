#include "Porosity.h"

#include "ExponentialPorosity.h"
#include "SoilMechanicsPorosity.h"
#include "DoubleExponentialPorosity.h"
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


   double Porosity::Algorithm::minimumMechanicalPorosity( ) const
   {
	   return m_minimumMechanicalPorosity;
   }

   double Porosity::Algorithm::surfacePorosity() const
   {
      return m_depoPorosity;
   }
}
