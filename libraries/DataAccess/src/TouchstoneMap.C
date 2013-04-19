#include <assert.h>
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
#endif // sgi


#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/TouchstoneMap.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"

using namespace DataAccess;
using namespace Interface;

TouchstoneMap::TouchstoneMap (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
}


TouchstoneMap::~TouchstoneMap (void)
{
}

const Formation * TouchstoneMap::getFormation (void) const
{
   return m_projectHandle->findFormation (database::getFormationName (m_record));
}

const Surface * TouchstoneMap::getSurface (void) const
{
   return m_projectHandle->findSurface (database::getSurfaceName (m_record));
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

   const Property * property = m_projectHandle->findProperty (propertyName);
   if (!property)
   {
      return 0;
   }

   PropertyValueList * propertyValues = m_projectHandle->getPropertyValues (FORMATIONSURFACE, property, 0, 0, getFormation (), getSurface (), MAP);

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

void TouchstoneMap::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void TouchstoneMap::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "TouchstoneMap: ";
   buf << getTCFName ();
   buf << ", " << getCategory ();
   buf << ", " << getFormat ();
   buf << ", " << getSurface ()->getName ();
   buf << ", " << getFormation ()->getName ();
   buf << endl;
   
   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
