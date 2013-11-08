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
   const double cutOff = 0.0;
   const double maxPerm = 1000.0;
   double val = 0.0;
   
   if ( ves > cutOff) 
   {
     return std::min( maxPerm, shalePermeability (ves, maxVes) );
   } 
   else
   {
     double a = 0.0, b = 0.0;
     shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
     double perm = a * (ves - cutOff) + b;
     return std::min( maxPerm, perm);
   }
}

void
PermeabilityMudStone
   :: permeabilityDerivative( const double ves, const double maxVes, const double calculatedPorosity,
		   const double porosityDerivativeWrtVes, double & permeability, double & derivative ) const
{
   const double cutOff = 0.0;
   const double maxPerm = 1000.0;

   if (ves >= cutOff && ves0 <= maxVes )
   {
      shalePermeabilityAndDerivative(ves, maxVes, permeability, derivative);
      permeability = std::min( maxPerm, permeability);
   }
   else if (ves >= cutOff && ves0 > maxVes)
   {
      double unused = 0.0;
      permeability = shalePermeability( ves, maxVes );
      shalePermeabilityAndDerivative( ves, ves0, unused, derivative);
   }
   else if (ves < cutOff && ves0 <= maxVes)
   {
     double a = 0.0, b = 0.0;
     shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
     permeability = std::min( a * (ves - cutOff) + b, maxPerm);

     double unused = 0.0;
     shalePermeabilityAndDerivative(ves, maxVes, unused, derivative);
   }
   else
   { // assert( ves <= cutOff && ves0 > maxVes )
     double a = 0.0, b = 0.0;
     shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
     permeability = std::min( a * (ves - cutOff) + b, maxPerm);

     double unused = 0.0;
     shalePermeabilityAndDerivative(cutOff, ves0, unused, derivative);
   }
}

double 
PermeabilityMudStone
   :: shalePermeability(  const double ves, const double maxVes) const
{
  assert ( 0 != ves0 );

  if (ves >= maxVes) 
  {
    return m_depoPermeability * fastPow(ves0/(ves+ves0), m_permeabilityIncr);
  } 
  else 
  {
    assert ( maxVes != -ves0 );
    assert ( 0 != maxVes );

    return m_depoPermeability * fastPow(ves0/(maxVes+ves0), m_permeabilityIncr) *
          fastPow((maxVes+ves0)/(ves+ves0), m_permeabilityDecr);
  }
}

void
PermeabilityMudStone
   :: shalePermeabilityAndDerivative( const double ves, const double maxVes, double & permeability, double & permeabilityDerivative) const
{
   assert (0 != ves0);
   assert (0 != (maxVes+ves0));

   permeability = shalePermeability(ves, maxVes);

   if (ves >= maxVes) 
   {
      permeabilityDerivative = -permeability * m_permeabilityIncr / (ves0 + ves);
   } 
   else
   {
      assert (maxVes != -ves0);
      permeabilityDerivative = -permeability * m_permeabilityDecr / (ves0 + ves);
   }
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

inline double
PermeabilityMudStone
   :: fastPow( double x, double y)
{
   if (y == 1.5)
   {
      // A favourite setting of m_permeabilityIncr is 1.5. It is the PermIncrRelaxCoef property of a lithology.
      return x * std::sqrt(x);
   }
   else 
   {
      return std::pow(x, y);
   }
}

}
