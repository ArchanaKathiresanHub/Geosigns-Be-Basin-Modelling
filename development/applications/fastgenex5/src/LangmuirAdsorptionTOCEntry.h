#ifndef _LANGMUIR_ADSORPTION_TOC_ENTRY_H_
#define _LANGMUIR_ADSORPTION_TOC_ENTRY_H_

#include "database.h"

class AdsorptionProjectHandle;

/// Class containing the contents a single line of the Langmuir adsorption isotherm table.
class LangmuirAdsorptionTOCEntry {

public :

   LangmuirAdsorptionTOCEntry ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                database::Record*        record );

   /// Return the temperature, C.
   double getReferenceTemperature () const;

   /// Return the temperature gradient.
   double getTemperatureGradient () const;

   /// Coefficient for the .
   double getCoeffA () const;

   /// Coefficient for the .
   double getCoeffB () const;

   /// Return the pressure, MPa.
   double getLangmuirPressure () const;

   /// Get the name of the formation to which the function is to be applied.
   const std::string& getLangmuirName () const;

   /// Return the string representation of the object.
   std::string image () const;

private :

   AdsorptionProjectHandle* m_adsorptionProjectHandle;
   double m_referenceTemperature;
   double m_temperatureGradient;
   double m_coeffA;
   double m_coeffB;
   double m_langmuirPressure;
   std::string m_langmuirName;

};

#endif // _LANGMUIR_ADSORPTION_TOC_ENTRY_H_
