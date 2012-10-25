/// Class LineAttributesList holds a list of LineAtributes, from which
/// LineAttributes can be retrieved
/// Struct LineAttributes holds color and width descriptions for a graph line

#include "lineattributes.h"
#include "globalstrings.h"
using namespace Graph_Properties;

// library references
#include "ComponentManager.h"
using namespace CBMGenerics;

//
// PUBLIC METHODS
//
LineAttributesList* LineAttributesList::getList ()
{
   static LineAttributesList m_list;
   return &m_list;
}

const LineAttributes* LineAttributesList::getLineAttributes (const char *lineName)
{
   LineAttributeMap::const_iterator lineIt = m_map.find (lineName);
   return (lineIt == m_map.end ()) ? NULL : &(lineIt->second);
}

/// LineAttributesList::getGeneralColour returns a colour from a list
/// of standard colours. They are used when any colour is required
/// for a line. When the list reaches its end, its index is set back to zero
const QColor& LineAttributesList::getGeneralColour ()
{
   if ( ++m_generalColourIndex == (short)m_generalColours.size() )
   {
      m_generalColourIndex = 0;
   }
   
   return m_generalColours [m_generalColourIndex];
}

//
// PRIVATE METHODS
//
/// LineAttributesList::LineAttributesList creates legend and colour
/// lists which will be used later
LineAttributesList::LineAttributesList ()
{
   createLegendList ();
   createGeneralColourList ();
   resetGeneralColourIndex ();
}

/// LineAttributesList::createLegendList creates legends for specific line graph
/// which need to have a specfic colour and or width
void LineAttributesList::createLegendList ()
{
   typedef pair <string, LineAttributes> InsertType;
   ComponentManager &tcm = ComponentManager::getInstance();

   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C1), LineAttributes (QColor (0, 255, 0), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C2), LineAttributes (QColor (48, 192, 48), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C3), LineAttributes (QColor (48, 146, 48), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C4), LineAttributes (QColor (150, 200, 0), 2))); 
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C5), LineAttributes (QColor (230, 225, 0), 2))); 
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::N2), LineAttributes (QColor (0, 128, 128), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::COx), LineAttributes (QColor (0, 200, 255), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C6Minus14Aro), LineAttributes (QColor (255, 188, 1), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C6Minus14Sat), LineAttributes (QColor (255,0,0), 2))); 
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C15PlusAro), LineAttributes (QColor (130, 0, 0), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::C15PlusSat), LineAttributes (QColor (255, 141, 27), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::resin), LineAttributes (QColor (200, 0, 0), 2)));
   m_map.insert (InsertType (tcm.GetSpeciesName(ComponentManager::asphaltene), LineAttributes (QColor (203, 150, 150), 2)));

   m_map.insert (InsertType (Pressure, LineAttributes (QColor (203, 150, 150), 2)));
   m_map.insert (InsertType (Temperature, LineAttributes (QColor (50, 198, 198), 2)));
   m_map.insert (InsertType (VolumeOil, LineAttributes (QColor (255, 0, 0), 2)));
   m_map.insert (InsertType (VolumeGas, LineAttributes (QColor (0, 255, 0), 2)));
   m_map.insert (InsertType (WetGas, LineAttributes (QColor (255, 150, 30), 3)));
   m_map.insert (InsertType (TotalGas, LineAttributes (QColor (0, 255, 0), 3)));
   m_map.insert (InsertType (TotalOil, LineAttributes (QColor (255, 0, 0), 3)));
   m_map.insert (InsertType (TrapCapacity, LineAttributes (QColor (0,0,0), 2)));      
}

void LineAttributesList::createGeneralColourList ()
{
   m_generalColours.push_back (QColor (200, 0, 0));
   m_generalColours.push_back (QColor (48, 146, 48));
   m_generalColours.push_back (QColor (0, 200, 255));
   m_generalColours.push_back (QColor (0, 255, 0));
   m_generalColours.push_back (QColor (48, 192, 48));
   m_generalColours.push_back (QColor (150, 200, 0));
   m_generalColours.push_back (QColor (230, 225, 0));
   m_generalColours.push_back (QColor (0, 128, 128));
   m_generalColours.push_back (QColor (255, 188, 1));
   m_generalColours.push_back (QColor (255, 0, 0));
}
