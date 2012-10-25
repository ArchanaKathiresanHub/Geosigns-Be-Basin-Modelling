/// Class NodeList is a stl::map of names associated with NodeEvent objects
/// The name part is a key made up of a layer name, an age, a trap Id or an x,y coordinate
/// It is created from the Source or Destination part of a row in the MigrationIoTbl
/// The NodeEvent object is an object which holds data about a specific migration event in time
/// including the source contributions to that event
/// This class also includes functions to handle minipulation of the node list

#ifndef __nodelist__
#define __nodelist__

#include "eventnode.h"

typedef map <string, EventNode> NodePtrList;
typedef NodePtrList::iterator NodePtrList_IT;
typedef NodePtrList::const_iterator const_NodePtrList_IT;

class NodeList
{
public:
   void addSrcToDestNode (const string& srcSRLayer, const string& srcReservoirLayer, const double srcAge, const int srcTrapId, 
                          const double srcX, const double srcY, const string& destLayer, const double destAge,
                          const int destTrapId, const double destX, const double destY, const double srcAmt);
   
   EventNode* addOrGetNode 
         (const string& key, const string& name); 
   EventNode* getNode 
         (const string& name, double age, int trapId, double x, double y);
   EventNode* getNode (const string& key);
   void printContents ();
   void clear ();
   int size () { return m_nodeList.size(); }
   void calculateSourcePercentages ();
   
   // static functions to create event node keys
   static string createEventKey (const string& layerName, const double age, const int trapId,
                                 const double x, const double y);
  
private:
   NodePtrList m_nodeList;
};



#endif
