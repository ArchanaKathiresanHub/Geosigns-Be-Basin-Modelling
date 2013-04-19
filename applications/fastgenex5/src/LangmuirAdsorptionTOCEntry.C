#include "LangmuirAdsorptionTOCEntry.h"
#include "AdsorptionProjectHandle.h"
#include <sstream>

#include "cauldronschemafuncs.h"

LangmuirAdsorptionTOCEntry::LangmuirAdsorptionTOCEntry ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                                         database::Record*        record ) : m_adsorptionProjectHandle ( adsorptionProjectHandle ) {

   m_referenceTemperature = database::getReferenceTemperature ( record );
   m_temperatureGradient = database::getVLTemperatureGradient ( record );
   m_coeffA = database::getVLCoeffA ( record );
   m_coeffB = database::getVLCoeffB ( record );
   m_langmuirPressure = database::getLangmuirPressure ( record );
   m_langmuirName = database::getLangmuirName ( record );
}

double LangmuirAdsorptionTOCEntry::getReferenceTemperature () const {
   return m_referenceTemperature;
} 

double LangmuirAdsorptionTOCEntry::getTemperatureGradient () const {
   return m_temperatureGradient;
} 

double LangmuirAdsorptionTOCEntry::getCoeffA () const {
   return m_coeffA;
} 

double LangmuirAdsorptionTOCEntry::getCoeffB () const {
   return m_coeffB;
} 

double LangmuirAdsorptionTOCEntry::getLangmuirPressure () const {
   return m_langmuirPressure;
} 

const std::string& LangmuirAdsorptionTOCEntry::getLangmuirName () const {
   return m_langmuirName;
} 

std::string LangmuirAdsorptionTOCEntry::image () const {

   std::stringstream buffer;

   buffer << " ( " 
          << getLangmuirName () << ", "
          << getReferenceTemperature () << ", " 
          << getCoeffA () << ", " 
          << getCoeffB () << ", " 
          << getTemperatureGradient () << ", " 
          << getLangmuirPressure () << " )";

   return buffer.str ();
}
