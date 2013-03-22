#include "PermeabilityMudStone.h"

#include <cmath>

namespace GeoPhysics
{

const double
PermeabilityMudStone
   :: s_log10 = std::log(10.0);

const double
PermeabilityMudStone
   :: ves0 = 1.0E+05;

PermeabilityMudStone
   :: PermeabilityMudStone( double depoPermeability, double permeabilityIncr, double permeabilityDecr)
   : m_depoPermeability( depoPermeability)
   , m_permeabilityIncr(permeabilityIncr)
   , m_permeabilityDecr(permeabilityDecr)
{}

double 
PermeabilityMudStone
   :: permeability( const double ves, const double maxVes, const double calculatedPorosity) const
{
   double cut_off = 0.0;
   double val = 0.0;
   
   if ( ves > cut_off) {
     val = shalePermeability (ves, maxVes);
   } else {
     double a = shalePermeabilityDerivative(cut_off, maxVes);
     double b = shalePermeability(cut_off, maxVes);
     val = a*(ves-cut_off)+b;
   }

   if (val >= 1000.0) val = 1000.0;

   return val;
}

void
PermeabilityMudStone
   :: permeabilityDerivative( const double ves, const double maxVes, const double calculatedPorosity,
                  double & permeability, double & derivative ) const
{
   permeability = this->permeability( ves, maxVes, calculatedPorosity);
   double maxVesUsed = maxVes;

   if ( maxVes < ves0 ) {
      maxVesUsed = ves0;
   }

   derivative = shalePermeabilityDerivative(ves, maxVesUsed);
}

double 
PermeabilityMudStone
   :: shalePermeability(  const double ves, const double maxVes) const
{
  double val;

  assert ( 0 != ves0 );

  if (ves >= maxVes) {
    val = m_depoPermeability * pow((ves+ves0)/ves0, -m_permeabilityIncr);
  } else {
    assert ( maxVes != -ves0 );
    assert ( 0 != maxVes );

    val = m_depoPermeability * pow ((maxVes+ves0)/ ves0, -m_permeabilityIncr) *
          pow ((ves+ves0)/(maxVes+ves0), -m_permeabilityDecr);
  }

  return val;
}

double
PermeabilityMudStone
   :: shalePermeabilityDerivative( const double ves, const double maxVes) const
{
   double val;

   assert (0 != ves0);
   assert (0 != (maxVes+ves0));

   if (ves >= maxVes) {
      val = -m_depoPermeability * m_permeabilityIncr * 
         pow ((ves+ves0)/ves0, (-m_permeabilityIncr - 1.0)) / ves0;
   } else {

      assert (maxVes != -ves0);
      val = -m_depoPermeability * m_permeabilityDecr * 
         pow (( maxVes + ves0) / ves0, -m_permeabilityIncr ) * 
         pow (( ves + ves0 ) / ( maxVes + ves0 ), 
              ( -m_permeabilityDecr - 1.0 )) / ( maxVes + ves0 );
   }

   return val;  
}

double
PermeabilityMudStone
   :: depoPerm() const
{
   return m_depoPermeability;
}

Permeability::Model
PermeabilityMudStone
   :: model() const
{
   return DataAccess::Interface::MUDSTONE_PERMEABILITY;
}
}
