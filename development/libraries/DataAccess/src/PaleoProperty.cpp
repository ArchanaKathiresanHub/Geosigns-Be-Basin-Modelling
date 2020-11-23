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
#include "Snapshot.h"
#include "PaleoProperty.h"

using namespace DataAccess;
using namespace Interface;



const string PaleoProperty::s_MapAttributeNames[] =
{
   "Thickness",   // Mobile layers.
   "HeatFlow",    // Bottom boundary condition.
   "Thickness",   // Crust thickness.
   "Thickness",   // Crust thickness.
   "Temperature", // Top surface temperature.
   "Depth",        // Top surface depth.
   "Thickness"   // Crust thickness.
};

PaleoProperty::PaleoProperty (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_snapshot = (const Snapshot *) projectHandle.findSnapshot (getAge (m_record), MAJOR );

   m_startProperty = 0;
   m_endProperty = 0;

}

PaleoProperty::PaleoProperty ( ProjectHandle&      projectHandle,
                               const Formation*     ,
                               const PaleoProperty* startProperty,
                               const PaleoProperty* endProperty,
                               const Snapshot*      interpolationTime ) :
   DAObject (projectHandle, 0),
   m_snapshot ( interpolationTime ),
   m_startProperty ( startProperty),
   m_endProperty ( endProperty ) {
}


PaleoProperty::~PaleoProperty (void)
{
}

const Snapshot * PaleoProperty::getSnapshot (void) const
{
   return m_snapshot;
}

const GridMap * PaleoProperty::getMap (PaleoPropertyMapAttributeId attributeId) const {

   unsigned int attributeIndex = (unsigned int) attributeId;
   GridMap * gridMap;

   if ((gridMap = (GridMap *) getChild (attributeIndex)) == 0)
   {
      gridMap = loadMap (attributeId);
   }

   return gridMap;
}

GridMap * PaleoProperty::loadMap (PaleoPropertyMapAttributeId attributeId) const {

   unsigned int attributeIndex = (unsigned int) attributeId;

   GridMap * gridMap = 0;

   if ( m_startProperty == 0 ) {
      const string attributeGridName = s_MapAttributeNames[attributeIndex] + "Grid";
      const string & valueGridMapId = m_record->getValue<std::string> (attributeGridName);

      if (valueGridMapId.length () != 0) {
         gridMap = getProjectHandle().loadInputMap ( m_record->getTable ()->name (), valueGridMapId);
      } else {
         double value= m_record->getValue<double>(s_MapAttributeNames[attributeIndex]);

         if (value != RecordValueUndefined)
         {
            const Grid * grid = getProjectHandle().getActivityOutputGrid();

            if (!grid) {
               grid = (Grid *) getProjectHandle().getInputGrid ();
            }

            gridMap = getProjectHandle().getFactory ()->produceGridMap (this, attributeIndex, grid, value);

            assert (gridMap == getChild (attributeIndex));
         }

      }

   } else {

      InterpolateFunctor interpolator ( m_startProperty->getSnapshot ()->getTime (),
                                        m_endProperty->getSnapshot ()->getTime (),
                                        m_snapshot->getTime ());

      gridMap = computeMap ( attributeId, m_startProperty->getMap ( attributeId ), m_endProperty->getMap ( attributeId ), interpolator );
   }

   return gridMap;
}


GridMap * PaleoProperty::computeMap ( const PaleoPropertyMapAttributeId attributeId,
                                      const GridMap * operand1,
                                      const GridMap * operand2,
                                      BinaryFunctor&  binaryFunctor) const {

   const unsigned int attributeInt = (unsigned int) ( attributeId );
   return getProjectHandle().getFactory ()->produceGridMap ( this, attributeInt, operand1, operand2, binaryFunctor );
}

bool DataAccess::Interface::PaleoPropertyTimeLessThan::operator ()( const PaleoProperty* s1, const PaleoProperty* s2 ) const {
   return s1->getSnapshot ()->getTime () < s2->getSnapshot ()->getTime ();
}

void PaleoProperty::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void PaleoProperty::asString (string & str) const
{
   ostringstream buf;

   buf << "PaleoProperty:";
   buf << " age = " << getSnapshot ()->getTime ();
   buf << endl;

   str = buf.str ();
}
