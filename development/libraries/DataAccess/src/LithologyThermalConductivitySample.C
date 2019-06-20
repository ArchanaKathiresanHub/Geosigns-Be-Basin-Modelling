#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Formation.h"
#include "LithologyThermalConductivitySample.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

LithologyThermalConductivitySample::LithologyThermalConductivitySample (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
#if 0
   m_lithotype = projectHandle->findLithoType ( database::getLithotype ( record ));
#endif
}

LithologyThermalConductivitySample::~LithologyThermalConductivitySample ()
{
}

#if 0
const LithoType* LithologyThermalConductivitySample::getLithoType () const
{
   return m_lithotype;
}
#endif

const std::string& LithologyThermalConductivitySample::getLithologyName () const {
   return database::getLithotype ( m_record );
}

double LithologyThermalConductivitySample::getTemperature () const
{
   return database::getTempIndex ( m_record );
}

double LithologyThermalConductivitySample::getThermalConductivity () const
{
   return database::getThCond ( m_record );
}


std::string LithologyThermalConductivitySample::image () const {

   std::ostringstream buffer;

   buffer << "LithologyThermalConductivitySample: " 
          << getLithologyName () << "  " 
          << getTemperature () << "  "
          << getThermalConductivity ();

   return buffer.str ();

}
