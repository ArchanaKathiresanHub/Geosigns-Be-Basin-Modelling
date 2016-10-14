//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PermeabilitySandStone.h"
#include "GeoPhysicalConstants.h"

// std library
#include <algorithm>
#include <cmath>

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::Log10;

namespace GeoPhysics
{

PermeabilitySandStone::PermeabilitySandStone( double depoPorosity, double depoPermeability, double permeabilityIncr)
   : m_depoPorosity( depoPorosity),
     m_depoPermeability( depoPermeability),
     m_permeabilityIncr( permeabilityIncr ),
     m_term ( 100.0 * Log10 * ( 0.12 + 0.02 * permeabilityIncr ))
{}

   inline double PermeabilitySandStone::calculateSingleValue ( const double calculatedPorosity ) const {
      double val = m_depoPermeability * exp ( m_term * ( calculatedPorosity - m_depoPorosity ));
      return std::min ( val, MaxPermeability );
   }

   inline void PermeabilitySandStone::calculateSingleValue ( const double porosity,
                                                             const double porosityDerivative,
                                                             double&      permeability,
                                                             double&      permeabilityDerivative ) const {

      permeability = calculateSingleValue ( porosity );


      if( permeability >= MaxPermeability )
      {
         permeabilityDerivative = 0.0;
      }
      else
      {
         // Use chainrule and multiply with derivative of porosity with respect to ves
         permeabilityDerivative = permeability * m_term * porosityDerivative;
      }

   }

   double PermeabilitySandStone::calculate( const double ves, const double maxVes, const double calculatedPorosity) const
   {
      // Added to prevent some compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;
      return calculateSingleValue ( calculatedPorosity );
   }

   void PermeabilitySandStone::calculate ( const unsigned int       n,
                                           ArrayDefs::ConstReal_ptr ves,
                                           ArrayDefs::ConstReal_ptr maxVes,
                                           ArrayDefs::ConstReal_ptr calculatedPorosity,
                                           ArrayDefs::Real_ptr      permeabilities ) const {

      // Added to prevent some compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;

      #pragma omp simd aligned (calculatedPorosity, permeabilities)
      for ( unsigned int i = 0; i < n; ++i ) {
         permeabilities [ i ] = calculateSingleValue ( calculatedPorosity [ i ]);
      }

   }

void PermeabilitySandStone::calculateDerivative( const double ves,
                                                 const double maxVes,
                                                 const double calculatedPorosity,
                                                 const double porosityDerivativeWrtVes,
                                                 double & permeability,
                                                 double & derivative ) const
{
   // Added to prevent some compiler warnings about unused parameters.
   (void) ves;
   (void) maxVes;

   calculateSingleValue ( calculatedPorosity, porosityDerivativeWrtVes, permeability, derivative );
}

   void PermeabilitySandStone::calculateDerivative ( const unsigned int       n,
                                                     ArrayDefs::ConstReal_ptr ves,
                                                     ArrayDefs::ConstReal_ptr maxVes,
                                                     ArrayDefs::ConstReal_ptr calculatedPorosity,
                                                     ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                                     ArrayDefs::Real_ptr      permeabilities,
                                                     ArrayDefs::Real_ptr      derivatives ) const {

      // Added to prevent some compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;

      #pragma omp simd aligned (calculatedPorosity, porosityDerivativeWrtVes, permeabilities, derivatives)
      for ( unsigned int i = 0; i < n; ++i ) {
         calculateSingleValue ( calculatedPorosity [ i ],
                                porosityDerivativeWrtVes [ i ],
                                permeabilities [ i ],
                                derivatives [ i ]);
      }

   }

double PermeabilitySandStone::depoPerm() const
{
   return m_depoPermeability;
}

Permeability::Model PermeabilitySandStone::model() const
{
   return DataAccess::Interface::SANDSTONE_PERMEABILITY;
}

}
