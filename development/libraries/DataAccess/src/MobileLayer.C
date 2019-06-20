#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"


#include "ProjectHandle.h"
#include "DAObject.h"
#include "ObjectFactory.h"
#include "GridMap.h"
#include "Formation.h"
#include "Snapshot.h"
#include "MobileLayer.h"

using namespace database;
using namespace DataAccess;
using namespace Interface;

MobileLayer::MobileLayer (ProjectHandle * projectHandle, Record * record) : 
   Interface::PaleoFormationProperty (projectHandle, record, 
                                           projectHandle->findFormation ( database::getLayerName (record)))
{
//    m_snapshot = (const Snapshot *) m_projectHandle->findSnapshot (getAge (m_record));
//    m_formation = (const Formation *) m_projectHandle->findFormation (getFormationName ());
}

MobileLayer::~MobileLayer (void)
{
}

// const string MobileLayer::s_MapAttributeNames[] =
// {
//    "Thickness"
// };

const string & MobileLayer::getFormationName (void) const
{
   return database::getLayerName (m_record);
}

const Formation * MobileLayer::getFormation (void) const
{
   return m_formation;
}

/// The present day thickness map of a mobile layer specification must be retrieved from the Formation
const GridMap * MobileLayer::getMap (PaleoPropertyMapAttributeId attributeId) const {
   if (attributeId == MobileLayerThicknessMap && getSnapshot()->getTime () == 0)
   {
      return getFormation()->getInputThicknessMap ();
   }
   else
   {
      return PaleoFormationProperty::getMap(attributeId);
   }
}


// const Snapshot * MobileLayer::getSnapshot (void) const
// {
//    return m_snapshot;
// }

// /// Return the (GridMap) value of one of this MobileLayer's attributes
// const GridMap * MobileLayer::getMap (MobileLayerMapAttributeId attributeId) const
// {
//    unsigned int attributeIndex = (unsigned int) attributeId;
//    GridMap * gridMap;

//    if ((gridMap = (GridMap *) getChild (attributeIndex)) == 0)
//    {
//       gridMap = loadMap (attributeId);
//    }
//    return gridMap;
// }

#if 0
const INTERFACE::GridMap * MobileLayer::getMap (INTERFACE::PaleoPropertyMapAttributeId attributeId) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;
   const INTERFACE::GridMap * gridMap;

   if ((gridMap = (GridMap *) getChild (attributeIndex)) == 0)
   {

      if ( database::getAge ( m_record ) == 0.0 ) {
         gridMap = getFormation ()->getInputThicknessMap ();
      } else {
         gridMap = PaleoProperty::getMap ( attributeId );
      }

   }

   return gridMap;
}
#endif
 
// GridMap * MobileLayer::loadMap (MobileLayerMapAttributeId attributeId) const
// {
//    unsigned int attributeIndex = (unsigned int) attributeId;

//    string attributeGridName = s_MapAttributeNames[attributeIndex] + "Grid";
//    const string & valueGridMapId = m_record->getValue (attributeGridName, (string *) 0);

//    GridMap * gridMap = 0;
//    if (valueGridMapId.length () != 0)
//    {
//       gridMap = m_projectHandle->loadInputMap ("MobLayThicknIoTbl", valueGridMapId);
//    }
//    else
//    {
//       double value;
//       if ((value = m_record->getValue (s_MapAttributeNames[attributeIndex], (double *) 0)) != RecordValueUndefined)
//       {
// 	 const Grid * grid = m_projectHandle->getActivityOutputGrid();
// 	 if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
//          gridMap = m_projectHandle->getFactory ()->produceGridMap (this, attributeIndex, grid, value);

//          assert (gridMap == getChild (attributeIndex));
//       }
//    }
//    return gridMap;
// }

// void MobileLayer::printOn (ostream & ostr) const
// {
//    string str;
//    asString (str);
//    ostr << str;
// }

void MobileLayer::asString (string & str) const
{
   ostringstream buf;

   buf << "MobileLayer:";
   buf << " age = " << getSnapshot ()->getTime ();
   buf << ", formation name = " << Interface::PaleoFormationProperty::getFormation ()->getName ();
   buf << endl;

   str = buf.str ();
}
