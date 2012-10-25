/// Class TrapSource takes a user-selected trap and searches for its origins
/// in an Event Tree. The Event Tree object will have been previously created through
/// the SourceReader

#ifndef __trapsource__
#define __trapsource__

class EventTree;
class EventNode;

#include "readonlydatabase.h" 
#include "global_typedefs.h"
using namespace PlotData;

#include <set>
using namespace std;

class TrapSource
{
public:
   TrapSource ();
   
   void setLineGroupPtr (LineGroup *ptr) { m_lineGroup = ptr; }
   LineGroup* getLineGroupPtr () { return m_lineGroup; }
   void generateSource (const string &resName, const double age, const int trapId, 
                     EventTree *eventTree, LineGroup *linePtr);
      
private:
   // private methods
   void setTables ();
   void generateSourceData ();
   void setData (const string &resName, const double age, const int trapId, 
                     EventTree *eventTree);
   EventNode* getUserNode ();
   LineGroupIT addLineGroup (LineGroup* lineGroup, const string &name, int size);
   void addTrapSourceToGraph (const EventNode *trap);
   void addToLineGroup (const string &lineName, double x, double y);
   void setTrapTotalAmt (EventNode *trap);
   double getSourceAge (const string& srcName);
      
   // private variables
   string m_resName;
   double m_age;
   double m_negativeAge;
   int m_trapId;
   EventNode *m_topNode;
   EventTree *m_eventTree;
   LineGroup *m_lineGroup;
   Table *m_trapIoTbl;
   Table *m_stratIoTbl;
};

#endif
