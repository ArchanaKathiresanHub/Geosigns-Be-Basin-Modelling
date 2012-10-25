/// Class EventTree creates and holds an EventNode list
/// It provides functions to populate the list and
/// retrieve information from it

#include "eventtree.h"
#include "componentsreader.h"

//
// PUBLIC METHODS
//

/// EventTree::readRecord takes a record from the migrationIoTbl
/// and extracts its source and destination information. It then
/// stores or adds this information in the EventNode List
void EventTree::readRecord (Record *rec)
{
   // add destination node, source node and source contribution to node tree
   string srcReservoirLayer = getSourceReservoirName (rec);           
   string srcSRLayer        = getSourceRockName(rec);

   m_nodeTree.addSrcToDestNode (
         srcSRLayer,
         srcReservoirLayer,
         getSourceAge (rec),
         getSourceTrapID (rec), 
         getSourcePointX (rec),
         getSourcePointY (rec),
         getDestinationReservoirName (rec), 
         getDestinationAge (rec), 
         getDestinationTrapID (rec), 
         getDestinationPointX (rec),
         getDestinationPointY (rec),
         getComponentValue (rec));
}

void EventTree::calculatePercents ()
{
   // calculate source node percentages of destination node total amount
   m_nodeTree.calculateSourcePercentages ();
}

EventNode* EventTree::findNode (const string& resName, 
                                      const double age, const int trapId,  const double x, const double y)
{
   return m_nodeTree.getNode (resName, age, trapId, x, y);
}

void EventTree::printContents ()
{
   m_nodeTree.printContents ();
}

//
// PRIVATE METHODS
//
double EventTree::getComponentValue (Record *rec)
{
   switch ( m_selectedComponent )
   {
   case 0: return getMassC1 (rec);
   case 1: return getMassC2 (rec);
   case 2: return getMassC3 (rec);
   case 3: return getMassC4 (rec);
   case 4: return getMassC5 (rec); 
   case 5: return getMassN2 (rec);
   case 6: return getMassCOx (rec);
   case 7: return getMassC6_14Aro (rec);
   case 8: return getMassC6_14Sat (rec);
   case 9: return getMassC15Aro (rec);
   case 10: return getMassC15Sat (rec);
   case 11: return getMassresins (rec);
   case 12: return getMassasphaltenes (rec);
   case 13: return ComponentsReader::getWetGas (rec);
   case 14: return ComponentsReader::getTotalGas (rec);
   case 15: return ComponentsReader::getTotalOil (rec);
   default: return 0;
   }
}

