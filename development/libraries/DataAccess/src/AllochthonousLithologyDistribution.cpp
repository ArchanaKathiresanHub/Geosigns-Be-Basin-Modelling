#include <assert.h>

#include <iostream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "GridMap.h"
#include "Formation.h"
#include "Snapshot.h"
#include "AllochthonousLithologyDistribution.h"

using namespace DataAccess;
using namespace Interface;

AllochthonousLithologyDistribution::AllochthonousLithologyDistribution (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{

   m_snapshot = (const Interface::Snapshot *) projectHandle.findSnapshot ( database::getAge (m_record));
   m_formation = (const Interface::Formation *) projectHandle.findFormation (getFormationName ());
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
   const string & valueGridMapId = m_record->getValue<std::string>(attributeGridName);

   GridMap * gridMap = 0;
   if (valueGridMapId.length () != 0)
   {
      gridMap = getProjectHandle().loadInputMap ("AllochthonLithoDistribIoTbl", valueGridMapId);
   }
   else
   {
      double value = m_record->getValue<double>(s_MapAttributeNames[attributeIndex]);
      if (value != RecordValueUndefined)
      {
         const Grid *grid = (Grid *) getProjectHandle().getInputGrid ();
         gridMap = getProjectHandle().getFactory ()->produceGridMap (this, attributeIndex, grid, value);

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

bool AllochthonousLithologyDistributionTimeLessThan::operator ()( const AllochthonousLithologyDistribution* ald1,
                                                                  const AllochthonousLithologyDistribution* ald2 ) const {
   return ald1->getAge () < ald2->getAge ();
}
