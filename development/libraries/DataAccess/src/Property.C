#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "Property.h"
#include "ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

Property::Property (ProjectHandle * projectHandle, database::Record * record,
                    const string & userName, const string & cauldronName, const string & unit,
                    PropertyType type,
                    const DataModel::PropertyAttribute attr, const DataModel::PropertyOutputAttribute outputAttr) :
   DAObject (projectHandle, record),
   m_userName (userName), m_cauldronName (cauldronName), m_unit (unit), m_type (type),
   m_propertyAttribute ( attr ), m_propertyOutputAttribute ( outputAttr ) 
{
   m_isPrimaryProperty = projectHandle->isPrimaryProperty( userName );
}

Property::~Property (void)
{
}

bool Property::hasPropertyValues (int selectionFlags,
      const Snapshot * snapshot,
      const Reservoir * reservoir, const Formation * formation,
      const Surface * surface, int propertyType) const
{
   return m_projectHandle->hasPropertyValues (selectionFlags, this, snapshot,
      reservoir, formation, surface, propertyType); 
}

const string & Property::getName (void) const
{
   return m_userName;
}

const string & Property::getUserName (void) const
{
   return m_userName;
}

const string & Property::getCauldronName (void) const
{
   return m_cauldronName;
}

const string & Property::getUnit (void) const
{
   return m_unit;
}

PropertyType Property::getType (void) const
{
   return m_type;
}

DataModel::PropertyAttribute Property::getPropertyAttribute () const {
   return m_propertyAttribute;
}

DataModel::PropertyOutputAttribute Property::getPropertyOutputAttribute () const {
   return m_propertyOutputAttribute;
}

PropertyValueList * Property::getPropertyValues (int selectionFlags,
      const Snapshot * snapshot,
      const Reservoir * reservoir, const Formation * formation,
      const Surface * surface) const
{
   return m_projectHandle->getPropertyValues (selectionFlags,
      this, snapshot, reservoir, formation, surface, SURFACE | VOLUME);
}

bool Property::isPrimary() const {

   return m_isPrimaryProperty;
}

void Property::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
   ostr << endl;

}

void Property::asString (string & str) const
{

   ostringstream buf;

   buf << "Property: ";
   buf << " name = " << getName ();
   buf << ", unit = " << getUnit ();
   buf << ", type = " << (getType () == FORMATIONPROPERTY ? "FORMATIONPROPERTY" : "RESERVOIRPROPERTY");

   str = buf.str ();
}
