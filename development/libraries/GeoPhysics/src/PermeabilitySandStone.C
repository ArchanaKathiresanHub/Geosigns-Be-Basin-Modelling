#include "PermeabilitySandStone.h"

#include <cmath>

namespace GeoPhysics
{

const double
PermeabilitySandStone
   :: s_log10 = std::log(10.0);

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
   double val = m_depoPermeability * pow(10.0, m * deltaphi * 100.0);

   if (val >= 1000.0) val = 1000.0;

   return val;
}

void
PermeabilitySandStone
   :: permeabilityDerivative( const double ves, const double maxVes, const double calculatedPorosity,
                  double & permeability, double & derivative ) const
{
   permeability = this->permeability( ves, maxVes, calculatedPorosity);

   double deltaphi = calculatedPorosity - m_depoPorosity;
   double m = 0.12 + 0.02 * m_permeabilityIncr;

   double perm =  m_depoPermeability * pow ( 10.0, m * deltaphi * 100.0);

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
