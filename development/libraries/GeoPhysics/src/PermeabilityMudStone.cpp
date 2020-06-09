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
#include <algorithm>

namespace GeoPhysics
{

   PermeabilityMudStone::PermeabilityMudStone( double depoPermeability, double permeabilityIncr, double permeabilityDecr)
      : m_depoPermeability( depoPermeability)
      , m_permeabilityIncr(permeabilityIncr)
      , m_permeabilityDecr(permeabilityDecr)
   {}

   double PermeabilityMudStone::calculate( const double ves, const double maxVes, const double calculatedPorosity) const
   {
      // Added to prevent a compiler warning about an unused parameter.
      (void) calculatedPorosity;

      const double cutOff = 0.0;

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
        return std::min( MaxPermeability, shalePermeability (ves, maxVes) );
      }
      else
      {
        double a = 0.0, b = 0.0;
        shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
        double perm = a * (ves - cutOff) + b;
        return std::min( MaxPermeability, perm);
      }
   }

   void PermeabilityMudStone::calculate ( const unsigned int       n,
                                          ArrayDefs::ConstReal_ptr ves,
                                          ArrayDefs::ConstReal_ptr maxVes,
                                          ArrayDefs::ConstReal_ptr calculatedPorosity,
                                          ArrayDefs::Real_ptr      permeabilities ) const {

      for ( unsigned int i = 0; i < n; ++i ) {
         permeabilities [ i ] = calculate ( ves [ i ], maxVes [ i ], calculatedPorosity [ i ]);
      }

   }


   void PermeabilityMudStone::calculateDerivative( const double ves,
                                                   const double maxVes,
                                                   const double calculatedPorosity,
                                                   const double porosityDerivativeWrtVes,
                                                   double & permeability,
                                                   double & derivative ) const
   {

      // Added to prevent a compiler warning about unused parameters.
      (void) calculatedPorosity;
      (void) porosityDerivativeWrtVes;

      const double cutOff = 0.0;

      // The reason for the check (ves >= cutoff) is to prevent the possibility of a nan or an inf
      // being returned from this permeability function.
      //
      // It does not appear in the other permeability functions (e.g Sandstone permeability)
      // because they do not depend directly on the ves. They are either a constant value
      // (None or Impermeable) or depend on the porosity (Sandstone or Multipoint).
      //
      // The ves can be negative during the Newton solve for the pressure
      // this is a temporary occurence.

#if WHICH_IS_FASTER
      if ( ves >= cutOff ) {

         if ( Ves0 <= maxVes ) {
            shalePermeabilityAndDerivative(ves, maxVes, permeability, derivative);

            if( permeability > MaxPermeability )
            {
               permeability = MaxPermeability;
               derivative   = 0.0;
            }

         } else {
            permeability = shalePermeability( ves, maxVes );

            if( permeability == MaxPermeability )
            {
               derivative = 0.0;
            }
            else
            {
               double unused = 0.0;
               shalePermeabilityAndDerivative( ves, Ves0, unused, derivative);
            }

         }

      } else {

         if ( Ves0 <= maxVes ) {
            double a = 0.0, b = 0.0;
            shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
            permeability = a * (ves - cutOff) + b;

            if( permeability > MaxPermeability )
            {
               permeability = MaxPermeability;
               derivative   = 0.0;
            }
            else
            {
               double unused = 0.0;
               shalePermeabilityAndDerivative(ves, maxVes, unused, derivative);
            }

         } else {
            double a = 0.0, b = 0.0;
            shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
            permeability = a * (ves - cutOff) + b;

            if( permeability > MaxPermeability )
            {
               permeability = MaxPermeability;
               derivative   = 0.0;
            }
            else
            {
               double unused = 0.0;
               shalePermeabilityAndDerivative(cutOff, Ves0, unused, derivative);
            }

         }

      }

#else
      if (ves >= cutOff && Ves0 <= maxVes )
      {
         shalePermeabilityAndDerivative(ves, maxVes, permeability, derivative);
         if( permeability > MaxPermeability )
         {
            permeability = MaxPermeability;
            derivative   = 0.0;
         }
      }
      else if (ves >= cutOff && Ves0 > maxVes)
      {
         permeability = shalePermeability( ves, maxVes );
         if( permeability == MaxPermeability )
         {
            derivative = 0.0;
         }
         else
         {
            double unused = 0.0;
            shalePermeabilityAndDerivative( ves, Ves0, unused, derivative);
         }
      }
      else if (ves < cutOff && Ves0 <= maxVes)
      {
         double a = 0.0, b = 0.0;
         shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
         permeability = a * (ves - cutOff) + b;
         if( permeability > MaxPermeability )
         {
            permeability = MaxPermeability;
            derivative   = 0.0;
         }
         else
         {
            double unused = 0.0;
            shalePermeabilityAndDerivative(ves, maxVes, unused, derivative);
         }
      }
      else
      {
         double a = 0.0, b = 0.0;
         shalePermeabilityAndDerivative(cutOff, maxVes, b, a);
         permeability = a * (ves - cutOff) + b;
         if( permeability > MaxPermeability )
         {
            permeability = MaxPermeability;
            derivative   = 0.0;
         }
         else
         {
            double unused = 0.0;
            shalePermeabilityAndDerivative(cutOff, Ves0, unused, derivative);
         }

      }

#endif
   }

   inline double PermeabilityMudStone::shalePermeability(  const double ves, const double maxVes) const
   {
     if (ves >= maxVes)
     {
       return m_depoPermeability * fastPow(Ves0/(ves+Ves0), m_permeabilityIncr);
     }
     else
     {
        //assert (0 != (ves+Ves0));    // this assert should be active, but what if during pressure solving we fall in this case?
        return m_depoPermeability * fastPow(Ves0/(maxVes+Ves0), m_permeabilityIncr) *
               fastPow((maxVes+Ves0)/(ves+Ves0), m_permeabilityDecr);
     }

   }

   inline void PermeabilityMudStone::shalePermeabilityAndDerivative( const double ves,
                                                                     const double maxVes,
                                                                     double & permeability,
                                                                     double & permeabilityDerivative ) const
   {
      //assert (0 != (ves+Ves0));    // this assert should be active, but what if during pressure solving we fall in this case?

      permeability = shalePermeability(ves, maxVes);

      if (ves >= maxVes)
      {
         permeabilityDerivative = -permeability * m_permeabilityIncr / (Ves0 + ves);
      }
      else
      {
         permeabilityDerivative = -permeability * m_permeabilityDecr / (Ves0 + ves);
      }
   }

   void PermeabilityMudStone::calculateDerivative ( const unsigned int       n,
                                                    ArrayDefs::ConstReal_ptr ves,
                                                    ArrayDefs::ConstReal_ptr maxVes,
                                                    ArrayDefs::ConstReal_ptr calculatedPorosity,
                                                    ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                                    ArrayDefs::Real_ptr      permeabilities,
                                                    ArrayDefs::Real_ptr      derivatives ) const {

      for ( unsigned int i = 0; i < n; ++i ) {
         calculateDerivative ( ves [ i ], maxVes [ i ],
                               calculatedPorosity [ i ], porosityDerivativeWrtVes [ i ],
                               permeabilities [ i ], derivatives [ i ]);
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

   inline double PermeabilityMudStone::fastPow( double x, double y )
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
