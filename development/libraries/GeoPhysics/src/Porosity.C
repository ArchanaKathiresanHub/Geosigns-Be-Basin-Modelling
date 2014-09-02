#include "Porosity.h"
#include "ExponentialPorosity.h"
#include "SoilMechanicsPorosity.h"
#include "Interface/Interface.h"
#include <sstream>

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
      Porosity::create(Model PorosityModel,
      double depoPorosity,
      double minimumMechanicalPorosity,
      double compactionincr,
      double compactiondecr,
      double soilMechanicsCompactionCoefficient)
   {
      switch (PorosityModel)
      {
      case DataAccess::Interface::EXPONENTIAL_POROSITY:
         return Porosity(new ExponentialPorosity(depoPorosity, minimumMechanicalPorosity, compactionincr, compactiondecr));
      case DataAccess::Interface::SOIL_MECHANICS_POROSITY:
         return Porosity(new soilMechanicsPorosity(depoPorosity, minimumMechanicalPorosity, soilMechanicsCompactionCoefficient, depoPorosity/(1-depoPorosity)));
      default:
         assert(false);
      }
      return Porosity(0);
   };
}
