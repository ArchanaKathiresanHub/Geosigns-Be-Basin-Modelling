/// Class NodeList is a stl::map of names associated with NodeEvent objects
/// The name part is a key made up of a layer name, an age, a trap Id or an x,y coordinate
/// It is created from the Source or Destination part of a row in the MigrationIoTbl
/// The NodeEvent object is an object which holds data about a specific migration event in time
/// including the source contributions to that event
/// This class also includes functions to handle minipulation of the node list

#include "nodelist.h"
#include "includestreams.h"

//
// PUBLIC METHODS
//
/// NodeList::addSrcToDestNode establishes a link between a destination and a source.
/// The SrcNode part of the Destination NodeEvent will point to the Source NodeEvent
void NodeList::addSrcToDestNode (const string& srcSRLayer, const string& srcReservoirLayer, const double srcAge, 
                                 const int srcTrapId, const double srcX, const double srcY, 
                                 const string& destLayer, const double destAge,
                                 const int destTrapId, const double destX, const double destY, 
                                 const double srcAmt)
{
   // create unique node keys
   string srcLayer, srcLayerForKey;
   if ( srcReservoirLayer.size() > 0 ) //is source formation reservoir or source rock?
   {
      srcLayer = srcLayerForKey = srcReservoirLayer;
   }
   else
   {
      srcLayer = srcLayerForKey = srcSRLayer;

      //make sure that source SR name and destination reservoir name used for key of migration process can be distinguished 
      //(otherwise one might get infinite recursion due to virtual trap -10 (expulsion from -10, leakage/spilling to -10))
      if ( srcLayer==destLayer )
      {
         srcLayerForKey += "(SR)";
      }
   }

   string srcKey = createEventKey (srcLayerForKey, srcAge, srcTrapId, srcX, srcY);
   string destKey = createEventKey (destLayer, destAge, destTrapId, destX, destY);
   
   if ( srcKey == destKey )
     return;

   // get or create source and destination nodes based on their keys
   EventNode *srcNode = addOrGetNode (srcKey, srcLayer); 
   EventNode *destNode = addOrGetNode (destKey, destLayer); 
   
   // add source node to destination node's list of source nodes
   destNode->addSourceNode (srcKey, srcNode, srcAmt);
}

/// NodeList::addOrGetNode adds a node, using a key to create it and gives it its source name
/// or retrieves the node if it already exists in the list
EventNode* NodeList::addOrGetNode (const string& key, const string& layerName) 
{  
   // find or create node in list
   EventNode *retNode = &(m_nodeList[key]);
   
   // if node just created then intialise 
   if ( ! retNode->initialised() )
   {
     retNode->init (layerName, key );
   }
                       
   return retNode;
}

EventNode* NodeList::getNode 
      (const string& name, double age, int trapId, double x, double y)
{
   return getNode (createEventKey (name, age, trapId, x, y));
}

EventNode* NodeList::getNode (const string& key)
{
   NodePtrList_IT nodeIt = m_nodeList.find (key);
   return  (nodeIt == m_nodeList.end()) ? NULL : &(nodeIt->second);
}

void NodeList::clear ()
{ 
   m_nodeList.erase (m_nodeList.begin(), m_nodeList.end());
}

void NodeList::calculateSourcePercentages ()
{
   NodePtrList_IT nodeIt, endNode = m_nodeList.end();
   for ( nodeIt = m_nodeList.begin(); nodeIt != endNode; ++nodeIt )
   {
      (nodeIt->second).calculatePercents ();
   }
}

string NodeList::createEventKey (const string& layerName, const double age, 
                                 const int trapId, const double x, const double y)
{
   char newKey [500];
   sprintf (newKey, "%s/%f/%d/%f/%f", layerName.c_str(), age, trapId, x, y);
   return newKey;
}

void NodeList::printContents ()
{
   cout << endl << "List of Source and Destination Nodes:" << endl;
   const_NodePtrList_IT nodeIt, endNode = m_nodeList.end();
   for ( nodeIt = m_nodeList.begin(); nodeIt != endNode; ++nodeIt )
   {
      cout << endl << "Node: " << nodeIt->first << endl;
      (nodeIt->second).printContents ();
   }
   cout << endl << "End list of Source and Destination Nodes:" << endl;
}
