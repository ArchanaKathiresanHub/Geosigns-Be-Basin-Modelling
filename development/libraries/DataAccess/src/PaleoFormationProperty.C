#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/PaleoFormationProperty.h"

using namespace DataAccess;
using namespace Interface;



#if 0
PaleoFormationProperty::PaleoFormationProperty (ProjectHandle * projectHandle, Record * record, const string& initialName ) : 
   PaleoProperty (projectHandle, record), 
   formationName ( initialName )
{
   m_formation = (const Formation *) m_projectHandle->findFormation ( getFormationName ());
}
#endif

PaleoFormationProperty::PaleoFormationProperty (ProjectHandle * projectHandle, Record * record, const Formation* formation ) : 
   PaleoProperty (projectHandle, record),
   formationName ( formation->getName ())
{
   m_formation = formation;
}

PaleoFormationProperty::PaleoFormationProperty ( ProjectHandle * projectHandle,
                                                 const Formation* formation,
                                                 const PaleoFormationProperty* startProperty,
                                                 const PaleoFormationProperty* endProperty,
                                                 const Snapshot*               interpolationTime ) :
   PaleoProperty ( projectHandle, formation, startProperty, endProperty, interpolationTime ),
   m_formation ( formation ),
   formationName ( formation->getName ()) {
}


PaleoFormationProperty::~PaleoFormationProperty (void)
{
}

const std::string& PaleoFormationProperty::getFormationName (void) const
{
   return formationName;
}

const Formation * PaleoFormationProperty::getFormation (void) const
{
   return m_formation;
}

void PaleoFormationProperty::asString (string & str) const
{
   ostringstream buf;

   buf << "PaleoFormationProperty:";
   buf << " age = " << getSnapshot ()->getTime ();
   buf << ", formation name = " << getFormation ()->getName ();
   buf << endl;

   str = buf.str ();
}
