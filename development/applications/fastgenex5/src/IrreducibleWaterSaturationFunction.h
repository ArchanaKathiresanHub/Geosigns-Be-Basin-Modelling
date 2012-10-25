#ifndef _IRREDUCIBLE_WATER_SATURATION_FUNCTION_H_
#define _IRREDUCIBLE_WATER_SATURATION_FUNCTION_H_

#include <cmath>

#include "AdsorptionProjectHandle.h"
#include "IrreducibleWaterSaturationSample.h"
#include "NumericFunctions.h"

class IrreducibleWaterSaturationFunction {

public :

   IrreducibleWaterSaturationFunction ( AdsorptionProjectHandle* adsorptionProjectHandle );

   /// Computes the irreducible-water-saturation.
   ///
   /// The value is capped at 1.0.
   double evaluate ( const double permeability ) const;

private :

   AdsorptionProjectHandle* m_adsorptionProjectHandle;

   double m_coeffA;
   double m_coeffB;

};

inline double IrreducibleWaterSaturationFunction::evaluate ( const double permeability ) const {

   double iws;

   iws = 0.01 * std::pow ( 10.0, m_coeffA * std::log10 ( permeability ) + m_coeffB );
   return NumericFunctions::Minimum ( iws, 1.0 );
}

#endif // _IRREDUCIBLE_WATER_SATURATION_FUNCTION_H_
