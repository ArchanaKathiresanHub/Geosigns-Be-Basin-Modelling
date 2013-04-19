#ifndef _LANGMUIR_ADSORPTION_ISOTHERM_H_
#define _LANGMUIR_ADSORPTION_ISOTHERM_H_

#include "LangmuirAdsorptionIsothermSample.h"

/// The function corresponding to a single line of isothermal values in the adsorption-project file.
class LangmuirAdsorptionIsotherm {

public :

   LangmuirAdsorptionIsotherm ( const LangmuirAdsorptionIsothermSample& sample );

   /// Compute the capacity of the isotherm.
   double compute ( const double temperature,
                    const double pressure ) const;

private :

   const LangmuirAdsorptionIsotherm& m_sample;

   const double m_temperature;
   const double m_pressure;
   const double m_volume;

};

#endif // _LANGMUIR_ADSORPTION_ISOTHERM_H_
