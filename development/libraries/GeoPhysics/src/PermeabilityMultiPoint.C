#include "PermeabilityMultiPoint.h"

#include <cmath>

namespace GeoPhysics
{

const double
PermeabilityMultiPoint
   :: s_log10 = std::log(10.0);

PermeabilityMultiPoint
   :: PermeabilityMultiPoint(double depoPorosity, const std::vector<double> & porositySamples, const std::vector<double> & permeabilitySamples)
   : m_porosityPermeabilityInterpolant()
   , m_depoPermeability(0)
{
   assert( porositySamples.size() == permeabilitySamples.size());
   assert( porositySamples.size() > 0);

   const int n = static_cast<int>(porositySamples.size());

   m_porosityPermeabilityInterpolant.setInterpolation(
         ibs::PiecewiseInterpolator::PIECEWISE_LINEAR,
         n,
         &porositySamples[0],
         &permeabilitySamples[0]
       );
         
   m_porosityPermeabilityInterpolant.computeCoefficients ();

   m_depoPermeability = m_porosityPermeabilityInterpolant.evaluate ( depoPorosity );
}

double 
PermeabilityMultiPoint
   :: permeability( const double ves, const double maxVes, const double calculatedPorosity) const
{
   double  val = exp ( s_log10 * m_porosityPermeabilityInterpolant.evaluate ( calculatedPorosity ));
   return std::min( val, 1000.0 );
}

void
PermeabilityMultiPoint
   :: permeabilityDerivative( const double ves, const double maxVes, const double calculatedPorosity,
		   const double porosityDerivativeWrtVes, double & permeability, double & derivative ) const
{
   permeability = this->permeability( ves, maxVes, calculatedPorosity);
   derivative = s_log10 * m_porosityPermeabilityInterpolant.evaluateDerivative ( calculatedPorosity ) * permeability;
   derivative *= porosityDerivativeWrtVes;
}

double
PermeabilityMultiPoint
   :: depoPerm() const
{
   return m_depoPermeability;
}

Permeability::Model
PermeabilityMultiPoint
   :: model() const
{
   return DataAccess::Interface::MULTIPOINT_PERMEABILITY;
}

}
