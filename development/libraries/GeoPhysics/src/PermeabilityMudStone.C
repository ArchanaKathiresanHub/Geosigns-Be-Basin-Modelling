//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PermeabilityMudStone.h"

#include "GeoPhysicalConstants.h"

#include <cmath>

namespace GeoPhysics
{

PermeabilityMudStone::PermeabilityMudStone( double depoPermeability, double permeabilityIncr, double permeabilityDecr)
   : m_depoPermeability( depoPermeability)
   , m_permeabilityIncr(permeabilityIncr)
   , m_permeabilityDecr(permeabilityDecr)
{}

double PermeabilityMudStone::calculate( const double ves, const double maxVes, const double calculatedPorosity) const
{
   const double cutOff = 0.0;
   const double maxPerm = 1000.0;
   double val = 0.0;

   // The reason for the check (ves > cutoff) is to prevent the possibility of a nan or an inf 
   // being returned from this permeability function.
   //
   // It does not appear in the other permeability functions (e.g Sandstone permeability)
   // because they do not depend directly on the ves. They are either a constant value
   // (None or Impermeable) or depend on the porosity (Sandstone or Multipoint).
   //
   // The ves can be negative during the Newton solve for the pressure 
   // this is a temporary occurence.
   if ( ves >= cutOff) 
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

void PermeabilityMudStone::calculateDerivative( const double ves, const double maxVes, const double calculatedPorosity,
		   const double porosityDerivativeWrtVes, double & permeability, double & derivative ) const
{
   const double cutOff = 0.0;
   const double maxPerm = 1000.0;

   // The reason for the check (ves >= cutoff) is to prevent the possibility of a nan or an inf 
   // being returned from this permeability function.
   //
   // It does not appear in the other permeability functions (e.g Sandstone permeability)
   // because they do not depend directly on the ves. They are either a constant value
   // (None or Impermeable) or depend on the porosity (Sandstone or Multipoint).
   //
   // The ves can be negative during the Newton solve for the pressure 
   // this is a temporary occurence.
   if (ves >= cutOff && Ves0 <= maxVes )
   {
      shalePermeabilityAndDerivative(ves, maxVes, permeability, derivative);
      permeability = std::min( maxPerm, permeability);
   }
   else if (ves >= cutOff && Ves0 > maxVes)
   {
      double unused = 0.0;
      permeability = shalePermeability( ves, maxVes );
      shalePermeabilityAndDerivative( ves, Ves0, unused, derivative );
   }
   else if (ves < cutOff && Ves0 <= maxVes)
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
     shalePermeabilityAndDerivative( cutOff, Ves0, unused, derivative );
   }
}

double PermeabilityMudStone::shalePermeability(  const double ves, const double maxVes) const
{
   assert( 0 != Ves0 );

  if (ves >= maxVes) 
  {
     return m_depoPermeability * fastPow( Ves0 / (ves + Ves0), m_permeabilityIncr );
  } 
  else 
  {
    assert( maxVes != -Ves0 );
    assert ( 0 != maxVes );

    return m_depoPermeability * fastPow( Ves0 / (maxVes + Ves0), m_permeabilityIncr ) *
       fastPow( (maxVes + Ves0) / (ves + Ves0), m_permeabilityDecr );
  }
}

void
PermeabilityMudStone::shalePermeabilityAndDerivative( const double ves, const double maxVes, double & permeability, double & permeabilityDerivative) const
{
   assert( 0 != Ves0 );
   assert( 0 != (maxVes + Ves0) );

   permeability = shalePermeability(ves, maxVes);

   if (ves >= maxVes) 
   {
      permeabilityDerivative = -permeability * m_permeabilityIncr / (Ves0 + ves);
   } 
   else
   {
      assert( maxVes != -Ves0 );
      permeabilityDerivative = -permeability * m_permeabilityDecr / (Ves0 + ves);
   }
}

double PermeabilityMudStone::depoPerm() const
{
   return m_depoPermeability;
}

Permeability::Model PermeabilityMudStone::model() const
{
   return DataAccess::Interface::MUDSTONE_PERMEABILITY;
}

inline double PermeabilityMudStone::fastPow( double x, double y)
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
