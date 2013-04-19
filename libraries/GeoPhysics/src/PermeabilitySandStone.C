#include "PermeabilitySandStone.h"

#include <cmath>

namespace GeoPhysics
{

const double
PermeabilitySandStone
   :: s_log10 = std::log(10.0);

const double
PermeabilitySandStone
   :: s_maxPerm = 1000.0;

PermeabilitySandStone
   :: PermeabilitySandStone( double depoPorosity, double depoPermeability, double permeabilityIncr)
   : m_depoPorosity( depoPorosity)
   , m_depoPermeability( depoPermeability)
   , m_permeabilityIncr(permeabilityIncr)
{}

double 
PermeabilitySandStone
   :: permeability( const double ves, const double maxVes, const double calculatedPorosity) const
{
   double deltaphi = calculatedPorosity - m_depoPorosity;
   double m = 0.12 + 0.02 * m_permeabilityIncr;
   double val = m_depoPermeability * exp(s_log10 * m * deltaphi * 100.0);

   if (val >= s_maxPerm ) val = s_maxPerm ;

   return val;
}

void
PermeabilitySandStone
   :: permeabilityDerivative( const double ves, const double maxVes, const double calculatedPorosity,
                  double & permeability, double & derivative ) const
{
   permeability = this->permeability( ves, maxVes, calculatedPorosity);

   double perm = permeability;
   double m = 0.12 + 0.02 * m_permeabilityIncr;

   if (perm >= s_maxPerm )
   {
      double deltaphi = calculatedPorosity - m_depoPorosity;

      perm =  m_depoPermeability * exp( s_log10 * m * deltaphi * 100.0);
   }
  
   // FIXME: Either it should be documented why the derivative is what it is now, or
   // this derivative is wrong.
   derivative = s_log10 * m * perm;  
}

double
PermeabilitySandStone
   :: depoPerm() const
{
   return m_depoPermeability;
}

Permeability::Model
PermeabilitySandStone
   :: model() const
{
   return DataAccess::Interface::SANDSTONE_PERMEABILITY;
}

}
