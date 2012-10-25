#include "LangmuirAdsorptionIsothermSample.h"

#include <sstream>

#include "AdsorptionProjectHandle.h"
#include "cauldronschemafuncs.h"


LangmuirAdsorptionIsothermSample::LangmuirAdsorptionIsothermSample ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                                                     database::Record*        record ) : m_adsorptionProjectHandle ( adsorptionProjectHandle ) {

   m_langmuirTemperature = database::getLangmuirTemperature ( record );
   m_langmuirPressure = database::getLangmuirPressure ( record );
   m_langmuirVolume = database::getLangmuirVolume ( record );
   m_langmuirName = database::getLangmuirName ( record );
}

double LangmuirAdsorptionIsothermSample::getLangmuirTemperature () const {
   return m_langmuirTemperature;
}

double LangmuirAdsorptionIsothermSample::getLangmuirPressure () const {
   return m_langmuirPressure;
}

double LangmuirAdsorptionIsothermSample::getLangmuirVolume () const {
   return m_langmuirVolume;
}

const std::string& LangmuirAdsorptionIsothermSample::getLangmuirName () const {
   return m_langmuirName;
}

std::string LangmuirAdsorptionIsothermSample::image () const {

   std::stringstream buffer;

   buffer << " ( " 
          << getLangmuirName () << ", "
          << getLangmuirTemperature () << ", " 
          << getLangmuirPressure () << ", "
          << getLangmuirVolume () << " )";

   return buffer.str ();
}
