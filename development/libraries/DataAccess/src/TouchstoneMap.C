#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;


#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "ProjectHandle.h"
#include "PropertyValue.h"
#include "TouchstoneMap.h"
#include "Surface.h"
#include "Formation.h"
#include "InputValue.h"

using namespace DataAccess;
using namespace Interface;

TouchstoneMap::TouchstoneMap (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{
}


TouchstoneMap::~TouchstoneMap (void)
{
}

const Formation * TouchstoneMap::getFormation (void) const
{
   return getProjectHandle().findFormation (database::getFormationName (m_record));
}

const Surface * TouchstoneMap::getSurface (void) const
{
   return getProjectHandle().findSurface (database::getSurfaceName (m_record));
}

/// Find the PropertyValue that was produced for this TouchstoneMap
const PropertyValue * TouchstoneMap::findPropertyValue (void) const
{
   string propertyName;
   propertyName += "Resq: ";
   propertyName += getTCFName ();
   propertyName += " ";
   propertyName += getCategory ();
   propertyName += " ";
   propertyName += getFormat ();

   const Property * property = getProjectHandle().findProperty (propertyName);
   if (!property)
   {
      return 0;
   }

   PropertyValueList * propertyValues = getProjectHandle().getPropertyValues (FORMATIONSURFACE, property, 0, 0, getFormation (), getSurface (), MAP);

   PropertyValueList::iterator propertyValueIter;

   const PropertyValue * propertyValue = 0;
   bool found = false;

   for (propertyValueIter = propertyValues->begin ();
        propertyValueIter != propertyValues->end () && !found; ++propertyValueIter)
   {
      propertyValue = *propertyValueIter;
      if (propertyValue->getName () == propertyName)
      {
   found = true;
      }
   }

   delete propertyValues;

   return found ? propertyValue : 0;
}


const string & TouchstoneMap::getTCFName (void) const
{
   return database::getTcfName (m_record);
}

const string & TouchstoneMap::getCategory (void) const
{
   return database::getCategory (m_record);
}

const string & TouchstoneMap::getFormat (void) const
{
   return database::getFormat (m_record);
}

double TouchstoneMap::getPercentage (void) const
{
   return database::getPercentile (m_record);
}

int TouchstoneMap::getFaciesNumber (void) const
{
   return database::getFaciesNumber (m_record);
}

const string & TouchstoneMap::getFaciesNameMap (void) const
{
   return database::getFaciesMap (m_record);
}

const GridMap * TouchstoneMap::getFaciesGridMap (void) const
{
	const Interface::InputValue * inputValue = getProjectHandle().findInputValue( "TouchstoneMapIoTbl", getFaciesNameMap( ) );

	//if FaciesMap is found return GridMap, otherwise return null
	return inputValue ? inputValue->getGridMap( ) : 0;
}

const string & TouchstoneMap::getRunName(void) const
{
	 return database::getRunName(m_record);
}

void TouchstoneMap::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void TouchstoneMap::asString (string & str) const
{
   ostringstream buf;

   buf << "TouchstoneMap: ";
   buf << getTCFName ();
   buf << ", " << getCategory ();
   buf << ", " << getFormat ();
   buf << ", " << getSurface ()->getName ();
   buf << ", " << getFormation ()->getName ();
   buf << endl;

   str = buf.str ();
}
