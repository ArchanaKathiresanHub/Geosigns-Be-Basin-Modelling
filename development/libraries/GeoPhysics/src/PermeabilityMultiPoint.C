//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PermeabilityMultiPoint.h"
#include "GeoPhysicalConstants.h"

#include <cmath>
#include <algorithm>

namespace GeoPhysics
{


   PermeabilityMultiPoint::PermeabilityMultiPoint(const double depoPorosity,
                                                  const std::vector<double> & porositySamples,
                                                  const std::vector<double> & permeabilitySamples)
      :m_porosityPermeabilityInterpolant(),
       m_depoPermeability( 0 )
   {
      assert( porositySamples.size() == permeabilitySamples.size());
      assert( porositySamples.size() > 0);

#if 1
      m_porosityPermeabilityInterpolant.setInterpolation ( porositySamples.size (), porositySamples.data (), permeabilitySamples.data ());
      m_depoPermeability = m_porosityPermeabilityInterpolant.evaluate ( depoPorosity );
#else
      // Scale the permeability values by log ( 10 ) so that this operation
      // does not need to be performed later when interpolating
      std::vector<double> scaledPermeabilitySamples ( permeabilitySamples );
      std::transform ( scaledPermeabilitySamples.begin (),
                       scaledPermeabilitySamples.end (),
                       scaledPermeabilitySamples.begin (),
                       [](const double x){ return Log10 * x; });
      m_porosityPermeabilityInterpolant.setInterpolation( porositySamples.size (), porositySamples.data (), scaledPermeabilitySamples.data ());

      // Need to divide by log ( 10 ) because interpolator has been scaled by log ( 10 ).
      m_depoPermeability = m_porosityPermeabilityInterpolant.evaluate ( depoPorosity ) / Log10;
#endif
   }

   inline double PermeabilityMultiPoint::calculateSingleValue ( const double porosity ) const {
      // return exp ( m_porosityPermeabilityInterpolant.evaluate ( porosity ));
      double val = exp ( m_porosityPermeabilityInterpolant.evaluate ( porosity ));
      return std::min( val, MaxPermeability );
   }


   double PermeabilityMultiPoint::calculate( const double ves, const double maxVes, const double calculatedPorosity ) const
   {

#if 1
      double  val = exp ( Log10 * m_porosityPermeabilityInterpolant.evaluate ( calculatedPorosity ));
      return std::min( val, MaxPermeability );
#else
      // Added to prevent compiler warning about unused parameters.
      (void) ves;
      (void) maxVes;

      return calculateSingleValue ( calculatedPorosity );
#endif
   }

   void PermeabilityMultiPoint::calculate ( const unsigned int       n,
                                            ArrayDefs::ConstReal_ptr ves,
                                            ArrayDefs::ConstReal_ptr maxVes,
                                            ArrayDefs::ConstReal_ptr calculatedPorosity,
                                            ArrayDefs::Real_ptr      permeabilities ) const {

      // Added to prevent some compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;

      // Initialise permeabilities array with the log of the permeabilty from the interpoaltor.
      m_porosityPermeabilityInterpolant.evaluate ( n, calculatedPorosity, permeabilities );

      // Now compute the permeability.
      #pragma simd
      for ( unsigned int i = 0; i < n; ++i ) {
         permeabilities [ i ] = std::min ( MaxPermeability, exp ( permeabilities [ i ] ));
      }

   }

   void PermeabilityMultiPoint::calculateDerivative( const double ves,
                                                     const double maxVes,
                                                     const double calculatedPorosity,
                                                     const double porosityDerivativeWrtVes,
                                                     double&      permeability,
                                                     double&      derivative ) const {

      // Added to prevent compiler warning about unused parameters.
      (void) ves;
      (void) maxVes;


#if 1
      permeability = calculate ( ves, maxVes, calculatedPorosity );

      if( permeability == MaxPermeability )
      {
         derivative = 0.0;
      }
      else
      {
         derivative = permeability * Log10 * porosityDerivativeWrtVes * m_porosityPermeabilityInterpolant.evaluateDerivative ( calculatedPorosity );
      }
#else
      permeability = calculateSingleValue ( calculatedPorosity );

      if ( permeability < MaxPermeability )
      {
         derivative = permeability * porosityDerivativeWrtVes * m_porosityPermeabilityInterpolant.evaluateDerivative ( calculatedPorosity );
      }
      else
      {
         derivative = 0.0;
      }
#endif

   }

   void PermeabilityMultiPoint::calculateDerivative ( const unsigned int       n,
                                                      ArrayDefs::ConstReal_ptr ves,
                                                      ArrayDefs::ConstReal_ptr maxVes,
                                                      ArrayDefs::ConstReal_ptr calculatedPorosity,
                                                      ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                                      ArrayDefs::Real_ptr      permeabilities,
                                                      ArrayDefs::Real_ptr      derivatives ) const {

      // Initialise permeabilities array with the log of the permeabilty.
      m_porosityPermeabilityInterpolant.evaluate ( n, calculatedPorosity, permeabilities, derivatives );

      #pragma simd
      for ( unsigned int i = 0; i < n; ++i ) {
         double perm = exp ( permeabilities [ i ]);

         if ( perm < MaxPermeability ) {
            permeabilities [ i ] = perm;
            derivatives [ i ] = perm * porosityDerivativeWrtVes [ i ] * derivatives [ i ];
         } else {
            permeabilities [ i ] = MaxPermeability;
            derivatives [ i ] = 0.0;
         }

      }

   }


   double PermeabilityMultiPoint::depoPerm() const
   {
      return m_depoPermeability;
   }

   Permeability::Model PermeabilityMultiPoint::model() const
   {
      return DataAccess::Interface::MULTIPOINT_PERMEABILITY;
   }

}
