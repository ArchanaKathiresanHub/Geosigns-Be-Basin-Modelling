#include "LangmuirAdsorptionIsothermSample.h"

#include <sstream>

#include "cauldronschemafuncs.h"

#include "database.h"

DataAccess::Interface::LangmuirAdsorptionIsothermSample::LangmuirAdsorptionIsothermSample ( ProjectHandle&    projectHandle,
                                                                                                 database::Record*  record ) : DAObject ( projectHandle, record ) {

   m_langmuirTemperature = database::getLangmuirTemperature ( record );
   m_langmuirPressure = database::getLangmuirPressure ( record );
   m_langmuirVolume = database::getLangmuirVolume ( record );
   m_langmuirName = database::getLangmuirName ( record );
}

DataAccess::Interface::LangmuirAdsorptionIsothermSample::LangmuirAdsorptionIsothermSample(DataAccess::Interface::ProjectHandle& projectHandle, const string& langmuirName, const double langmuirTemperature, const double langmuirPressure, const double langmuirVolume) :
  DAObject(projectHandle, nullptr),
  m_langmuirTemperature(langmuirTemperature),
  m_langmuirPressure(langmuirPressure),
  m_langmuirVolume(langmuirVolume),
  m_langmuirName(langmuirName)
{
}

DataAccess::Interface::LangmuirAdsorptionIsothermSample::~LangmuirAdsorptionIsothermSample ()
{
}

double DataAccess::Interface::LangmuirAdsorptionIsothermSample::getLangmuirTemperature () const {
   return m_langmuirTemperature;
}

double DataAccess::Interface::LangmuirAdsorptionIsothermSample::getLangmuirPressure () const {
   return m_langmuirPressure;
}

double DataAccess::Interface::LangmuirAdsorptionIsothermSample::getLangmuirVolume () const {
   return m_langmuirVolume;
}

const std::string&DataAccess::Interface:: LangmuirAdsorptionIsothermSample::getLangmuirName () const {
   return m_langmuirName;
}

std::string DataAccess::Interface::LangmuirAdsorptionIsothermSample::image () const {

   std::stringstream buffer;

   buffer << " LangmuirAdsorptionIsothermSample ( "
          << getLangmuirName () << ", "
          << getLangmuirTemperature () << ", "
          << getLangmuirPressure () << ", "
          << getLangmuirVolume () << " )";

   return buffer.str ();
}


bool DataAccess::Interface::LangmuirAdsorptionIsothermSampleLessThan::operator ()( const LangmuirAdsorptionIsothermSample* s1,
                                                                                        const LangmuirAdsorptionIsothermSample* s2 ) const {
   return s1->getLangmuirTemperature () < s2->getLangmuirTemperature ();
}
