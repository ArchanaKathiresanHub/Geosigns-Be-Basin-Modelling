/// Class EventTree creates and holds an EventNode list
/// It provides functions to populate the list and
/// retrieve information from it

#ifndef __eventtree__
#define __eventtree__

#include "nodelist.h"
#include "readonlydatabase.h"

class EventTree
{
public:
   EventTree (short comp=0)   
    : m_selectedComponent (comp), 
    m_tableSize (0)
   {}
   
   ~EventTree () { m_nodeTree.clear (); }

   int tableSize() { return m_tableSize; }
   void setTableSize (int size) { m_tableSize = size; }
   void readRecord (Record *rec);
   void calculatePercents ();
   void setSelectedComponent (short comp) { m_selectedComponent = comp; }
   EventNode* findNode (const string& resName, const double age, const int trapId,
                              const double x, const double y);
   void printContents ();
   int treeSize () { return m_nodeTree.size(); }
   double getComponentValue (Record *rec);
   void clear () { m_nodeTree.clear (); }
   
private: 
   NodeList m_nodeTree;
   short m_selectedComponent;
   int m_tableSize;
};

#endif
