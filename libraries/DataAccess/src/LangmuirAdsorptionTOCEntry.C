#include "Interface/LangmuirAdsorptionTOCEntry.h"
#include <sstream>

#include "cauldronschemafuncs.h"
#include "database.h"

DataAccess::Interface::LangmuirAdsorptionTOCEntry::LangmuirAdsorptionTOCEntry ( ProjectHandle*     projectHandle,
                                                                                     database::Record*  record ) : DAObject ( projectHandle, record ) {

   m_referenceTemperature = database::getReferenceTemperature ( record );
   m_temperatureGradient = database::getVLTemperatureGradient ( record );
   m_coeffA = database::getVLCoeffA ( record );
   m_coeffB = database::getVLCoeffB ( record );
   m_langmuirPressure = database::getLangmuirPressure ( record );
   m_langmuirName = database::getLangmuirName ( record );
}


DataAccess::Interface::LangmuirAdsorptionTOCEntry::~LangmuirAdsorptionTOCEntry () {
}

double DataAccess::Interface::LangmuirAdsorptionTOCEntry::getReferenceTemperature () const {
   return m_referenceTemperature;
} 

double DataAccess::Interface::LangmuirAdsorptionTOCEntry::getTemperatureGradient () const {
   return m_temperatureGradient;
} 

double DataAccess::Interface::LangmuirAdsorptionTOCEntry::getCoeffA () const {
   return m_coeffA;
} 

double DataAccess::Interface::LangmuirAdsorptionTOCEntry::getCoeffB () const {
   return m_coeffB;
} 

double DataAccess::Interface::LangmuirAdsorptionTOCEntry::getLangmuirPressure () const {
   return m_langmuirPressure;
} 

const std::string& DataAccess::Interface::LangmuirAdsorptionTOCEntry::getLangmuirName () const {
   return m_langmuirName;
} 

std::string DataAccess::Interface::LangmuirAdsorptionTOCEntry::image () const {

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
