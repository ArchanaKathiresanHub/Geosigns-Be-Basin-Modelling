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

#include "Interface/Property.h"
#include "Interface/ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

Property::Property (ProjectHandle * projectHandle, database::Record * record,
      const string & userName, const string & cauldronName, const string & unit,
      PropertyType type) :
   DAObject (projectHandle, record),
   m_userName (userName), m_cauldronName (cauldronName), m_unit (unit), m_type (type)
{
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

PropertyValueList * Property::getPropertyValues (int selectionFlags,
      const Snapshot * snapshot,
      const Reservoir * reservoir, const Formation * formation,
      const Surface * surface) const
{
   return m_projectHandle->getPropertyValues (selectionFlags,
      this, snapshot, reservoir, formation, surface, SURFACE | VOLUME);
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
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "Property: ";
   buf << " name = " << getName ();
   buf << ", unit = " << getUnit ();
   buf << ", type = " << (getType () == FORMATIONPROPERTY ? "FORMATIONPROPERTY" : "RESERVOIRPROPERTY");

//    buf << ", type = ";
//    switch ( getType ()) {
//      case FORMATIONPROPERTY : buf << "FORMATIONPROPERTY"; break;
//      case SURFACEPROPERTY   : buf << "SURFACEPROPERTY"; break;
//      case RESERVOIRPROPERTY : buf << "RESERVOIRPROPERTY"; break;
//    }


//    << (getType () == FORMATIONPROPERTY ? "FORMATIONPROPERTY" : ( getType () == SURFACEPROPERTY ? "SURFACEPROPERTY" : "RESERVOIRPROPERTY" )) << " " << int ( getType ());

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
