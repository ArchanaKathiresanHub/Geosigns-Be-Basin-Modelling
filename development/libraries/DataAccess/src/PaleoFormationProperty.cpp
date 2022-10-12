#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "GridMap.h"
#include "Formation.h"
#include "Snapshot.h"
#include "PaleoFormationProperty.h"

using namespace DataAccess;
using namespace Interface;

PaleoFormationProperty::PaleoFormationProperty (ProjectHandle& projectHandle, Record * record, const Formation* formation ) :
   PaleoProperty (projectHandle, record),
   formationName ( formation->getName ())
{
   m_formation = formation;
}

PaleoFormationProperty::PaleoFormationProperty ( ProjectHandle& projectHandle,
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
   std::ostringstream buf;

   buf << "PaleoFormationProperty:";
   buf << " age = " << getSnapshot ()->getTime ();
   buf << ", formation name = " << getFormation ()->getName ();
   buf << endl;

   str = buf.str ();
}
