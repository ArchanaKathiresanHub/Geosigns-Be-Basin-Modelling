/// Class SourceReader inhertits from DataReader and adds
/// specific functionality for reading component data
/// It also has its own componentsSelected variable that is set from 
/// the components graph when the user selectes a new components

#ifndef __sourcereader__
#define __sourcereader__

#include "datareader.h"
#include "trapsource.h"
#include "eventtree.h"

#include "globalstrings.h"
using namespace Graph_Properties;

class SourceReader : public DataReader
{
public:
   // public ctor / dtor
   SourceReader () : m_selectedComponent (0), DataReader () {}

   // public methods
   string readData(const char* filenme); // (override pure virtual base method)
   
   void setSelectedComponent (short c) { m_selectedComponent = c; }
   short getSelectedComponent (void) { return m_selectedComponent; }
 
private:
   // private methods (overriding base virtual methods)
   void readRecord (Record *rec);
   bool recordWanted (Record *rec) { return rec; }
   
   void generateTrapSourceForUserTrap ();

   // private variables
   short m_selectedComponent;
   EventTree m_eventTree;
   TrapSource m_trapSource;
};

#endif
