#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/Formation.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

LithologyHeatCapacitySample::LithologyHeatCapacitySample (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
#if 0
   m_lithotype = projectHandle->findLithoType ( database::getLithotype ( record ));
#endif
}


LithologyHeatCapacitySample::~LithologyHeatCapacitySample ()
{
}

const std::string& LithologyHeatCapacitySample::getLithologyName () const {
   return database::getLithotype ( m_record );
}

#if 0
const LithoType* LithologyHeatCapacitySample::getLithoType () const
{
   return m_lithotype;
}
#endif

double LithologyHeatCapacitySample::getTemperature () const
{
   return database::getTempIndex ( m_record );
}

double LithologyHeatCapacitySample::getHeatCapacity () const
{
   return database::getHeatCapacity ( m_record );
}


std::string LithologyHeatCapacitySample::image () const {

   std::ostringstream buffer;

   buffer << "LithologyHeatCapacitySample: " 
          << getLithologyName () << "  " 
          << getTemperature () << "  "
          << getHeatCapacity ();

   return buffer.str ();

}
