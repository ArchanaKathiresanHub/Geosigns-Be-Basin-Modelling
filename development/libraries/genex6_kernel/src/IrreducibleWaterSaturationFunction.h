#ifndef _GENEX6_KERNEL__IRREDUCIBLE_WATER_SATURATION_FUNCTION_H_
#define _GENEX6_KERNEL__IRREDUCIBLE_WATER_SATURATION_FUNCTION_H_

#include <cmath>

#include "ProjectHandle.h"
#include "IrreducibleWaterSaturationSample.h"

#include "NumericFunctions.h"

namespace Genex6 {

   /// \brief Functor for computing the irreducible water saturation.
   class IrreducibleWaterSaturationFunction {

   public :

      IrreducibleWaterSaturationFunction ( DataAccess::Interface::ProjectHandle& projectHandle );

      /// \brief Computes the irreducible-water-saturation.
      ///
      /// The value is capped at 1.0.
      double evaluate ( const double permeability ) const;

   private :

      double m_coeffA;
      double m_coeffB;

   };

}

inline double Genex6::IrreducibleWaterSaturationFunction::evaluate ( const double permeability ) const {

   double iws;

   iws = 0.01 * std::pow ( 10.0, m_coeffA * std::log10 ( permeability ) + m_coeffB );
   return NumericFunctions::Minimum ( iws, 1.0 );
}

#endif // _GENEX6_KERNEL__IRREDUCIBLE_WATER_SATURATION_FUNCTION_H_
