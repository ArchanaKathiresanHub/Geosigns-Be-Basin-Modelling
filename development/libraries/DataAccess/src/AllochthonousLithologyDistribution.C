#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/AllochthonousLithologyDistribution.h"

using namespace DataAccess;
using namespace Interface;

AllochthonousLithologyDistribution::AllochthonousLithologyDistribution (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{

   m_snapshot = (const Interface::Snapshot *) m_projectHandle->findSnapshot ( database::getAge (m_record));
   m_formation = (const Interface::Formation *) m_projectHandle->findFormation (getFormationName ());
}

AllochthonousLithologyDistribution::~AllochthonousLithologyDistribution (void)
{
}

const string AllochthonousLithologyDistribution::s_MapAttributeNames[] =
{
  "Distribution"
};

const string & AllochthonousLithologyDistribution::getFormationName (void) const
{
   return database::getLayerName (m_record);
}

double AllochthonousLithologyDistribution::getAge (void) const
{
  return database::getAge (m_record);
}

const Formation * AllochthonousLithologyDistribution::getFormation (void) const
{
   return m_formation;
}

const Snapshot * AllochthonousLithologyDistribution::getSnapshot (void) const
{
   return m_snapshot;
}

/// Return the (GridMap) value of one of this AllochthonousLithologyDistribution's attributes
const GridMap * AllochthonousLithologyDistribution::getMap ( const Interface::AllochthonousLithologyDistributionMapAttributeId attributeId ) const
{

   unsigned int attributeIndex = (unsigned int) attributeId;
   GridMap * gridMap;

   if ((gridMap = (GridMap *) getChild (attributeIndex)) == 0)
   {
      gridMap = loadMap (attributeId);
   }
   return gridMap;
}
 
GridMap * AllochthonousLithologyDistribution::loadMap ( const Interface::AllochthonousLithologyDistributionMapAttributeId attributeId ) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;

   string attributeGridName = s_MapAttributeNames[attributeIndex] + "Grid";
   const string & valueGridMapId = m_record->getValue (attributeGridName, (string *) 0);

   GridMap * gridMap = 0;
   if (valueGridMapId.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("AllochthonLithoDistribIoTbl", valueGridMapId);
   }
   else
   {
      double value;
      if ((value = m_record->getValue (s_MapAttributeNames[attributeIndex], (double *) 0)) != RecordValueUndefined)
      {
         const Grid *grid = (Grid *) m_projectHandle->getInputGrid ();
         gridMap = m_projectHandle->getFactory ()->produceGridMap (this, attributeIndex, grid, value);

         assert (gridMap == getChild (attributeIndex));
      }
   }
   return gridMap;
}

void AllochthonousLithologyDistribution::printOn (ostream & ostr) const
{
   ostr << "AllochthonousLithologyDistribution:";
   ostr << "  formation name = " << getFormation ()->getName ();
   ostr << ", age = " << getSnapshot ()->getTime ();
   ostr << endl;
}
