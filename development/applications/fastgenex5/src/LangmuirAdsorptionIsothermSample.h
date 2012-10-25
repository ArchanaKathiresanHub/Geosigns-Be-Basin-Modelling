#ifndef _LANGMUIR_ADSORPTION_ISOTHERM_SAMPLE_H_
#define _LANGMUIR_ADSORPTION_ISOTHERM_SAMPLE_H_

#include "database.h"

class AdsorptionProjectHandle;

/// Class containing the contents a single line of the Langmuir adsorption isotherm table.
class LangmuirAdsorptionIsothermSample {

public :

   LangmuirAdsorptionIsothermSample ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                      database::Record*        record );

   /// Return the temperature, C.
   double getLangmuirTemperature () const;

   /// Return the pressure, MPa.
   double getLangmuirPressure () const;

   /// Return the volume, cc/g.
   double getLangmuirVolume () const;

   /// Get the name of the isotherm set this isotherm belongs to
   const std::string& getLangmuirName () const;

   /// Return the string representation of the object.
   std::string image () const;


private :

   AdsorptionProjectHandle* m_adsorptionProjectHandle;
   double m_langmuirTemperature;
   double m_langmuirPressure;
   double m_langmuirVolume;
   std::string m_langmuirName;

};

#endif // _LANGMUIR_ADSORPTION_ISOTHERM_SAMPLE_H_
