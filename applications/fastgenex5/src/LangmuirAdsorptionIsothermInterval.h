#ifndef _LANGMUIR_ADSORPTION_ISOTHERM_INTERVAL_H_
#define _LANGMUIR_ADSORPTION_ISOTHERM_INTERVAL_H_

#include <string>

#include "LangmuirAdsorptionIsothermSample.h"

/// A simple Langmuir adsorption isotherm interval.
///
/// The V_L and P_L will be interpolated linearly between the lower and upper temperature values.
class LangmuirAdsorptionIsothermInterval {

public :

   LangmuirAdsorptionIsothermInterval ( const LangmuirAdsorptionIsothermSample* lowerBound,
                                        const LangmuirAdsorptionIsothermSample* upperBound );

   /// Get the lower bound sample.
   const LangmuirAdsorptionIsothermSample* getLowerBound () const;

   /// Get the upper bound sample.
   const LangmuirAdsorptionIsothermSample* getUpperBound () const;

   /// Indicate whether or not the temperature is in the range of the interval.
   bool temperatureInRange ( const double temperature ) const;

   /// Compute the Langmuir adsorption function for the interval.
   ///
   /// There is no check to ensure that the temperature lies in the correct range [ low, high ].
   double compute ( const double temperature,
                    const double pressure ) const;

   /// Compute VL for the interval.
   ///
   /// There is no check to ensure that the temperature lies in the correct range [ low, high ].
   double computeVL ( const double temperature ) const;

   /// Return the string representation of the interval.
   std::string image () const;

private :

   const LangmuirAdsorptionIsothermSample* m_lowerBound;
   const LangmuirAdsorptionIsothermSample* m_upperBound;

   const double m_temperatureLow;
   const double m_vlLow;
   const double m_plLow;

   const double m_temperatureHigh;
   const double m_vlHigh;
   const double m_plHigh;

   const double m_temperatureDiff;

   const double m_aCoeffVl;
   const double m_bCoeffVl;
   const double m_aCoeffPl;
   const double m_bCoeffPl;

};

inline const LangmuirAdsorptionIsothermSample* LangmuirAdsorptionIsothermInterval::getLowerBound () const {
   return m_lowerBound;
}

inline const LangmuirAdsorptionIsothermSample* LangmuirAdsorptionIsothermInterval::getUpperBound () const {
   return m_upperBound;
}


#endif // _LANGMUIR_ADSORPTION_ISOTHERM_INTERVAL_H_
