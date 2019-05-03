#ifndef _GENEX6_KERNEL__LANGMUIR_ADSORPTION_ISOTHERM_INTERVAL_H_
#define _GENEX6_KERNEL__LANGMUIR_ADSORPTION_ISOTHERM_INTERVAL_H_

#include <string>

#include "Interface/LangmuirAdsorptionIsothermSample.h"

namespace Genex6 {

   /// A simple Langmuir adsorption isotherm interval.
   ///
   /// The V_L and P_L will be interpolated linearly between the lower and upper temperature values.
   class LangmuirAdsorptionIsothermInterval {

   public :

      LangmuirAdsorptionIsothermInterval ( const DataAccess::Interface::LangmuirAdsorptionIsothermSample* lowerBound,
                                           const DataAccess::Interface::LangmuirAdsorptionIsothermSample* upperBound );

      /// Get the lower bound sample.
      const DataAccess::Interface::LangmuirAdsorptionIsothermSample* getLowerBound () const;

      /// Get the upper bound sample.
      const DataAccess::Interface::LangmuirAdsorptionIsothermSample* getUpperBound () const;

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

      /// \brief Indicate whether or not the interval has valid values.
      bool isValid () const;

      /// \brief Get information about any error that may have occurred during initialisation.
      std::string getErrorMessage () const;

   private :

      const DataAccess::Interface::LangmuirAdsorptionIsothermSample* m_lowerBound;
      const DataAccess::Interface::LangmuirAdsorptionIsothermSample* m_upperBound;

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

}

inline const DataAccess::Interface::LangmuirAdsorptionIsothermSample* Genex6::LangmuirAdsorptionIsothermInterval::getLowerBound () const {
   return m_lowerBound;
}

inline const DataAccess::Interface::LangmuirAdsorptionIsothermSample* Genex6::LangmuirAdsorptionIsothermInterval::getUpperBound () const {
   return m_upperBound;
}


#endif // _GENEX6_KERNEL__LANGMUIR_ADSORPTION_ISOTHERM_INTERVAL_H_
