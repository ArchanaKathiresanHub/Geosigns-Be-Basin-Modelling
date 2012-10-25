/// Class TrapSource takes a user-selected trap and searches for its origins
/// in an Event Tree. The Event Tree object will have been previously created through
/// the SourceReader

#include "trapsource.h"
#include "eventtree.h"

//
// PUBLIC METHODS
//
TrapSource::TrapSource () 
   : m_topNode (0), m_eventTree (0), m_lineGroup (0) 
{}

/// TrapSource::generateSource initiates searching through an EventTree for 
/// trap source data
void TrapSource::generateSource (const string &resName, const double age, 
                              const int trapId, 
                              EventTree *eventTree, LineGroup *linePtr)
{
   setTables ();
   setData (resName, age, trapId, eventTree);
   setLineGroupPtr (linePtr);
   generateSourceData ();
}

//
// PRIVATE METHODS
//
void TrapSource::setTables ()
{
   m_trapIoTbl = ReadOnlyDatabase::getReadOnlyDatabase ()->getTable ("TrapIoTbl");
   m_stratIoTbl = ReadOnlyDatabase::getReadOnlyDatabase ()->getTable ("StratIoTbl");
}

/// TrapSource::setData set user-selected data and initialise 
void TrapSource::setData (const string &resName, const double age, 
                              const int trapId, 
                              EventTree *eventTree)
{
   m_resName = resName;
   m_age = age; 
   m_negativeAge = age - (2 * age);
   m_trapId = trapId;
   m_eventTree = eventTree;
   m_topNode = 0;
}

/// TrapSource::generateSourceData gets a user-selected trap and 
/// initiates a recursive search for the traps source origins
void TrapSource::generateSourceData ()
{
   // get user selected trap
   EventNode *topTrap = getUserNode();
   
   // search for source recursively if any source exists
   if ( topTrap && (topTrap->numSourceNodes () > 0) )
   {
      topTrap->recursiveSearchForSource ();
      setTrapTotalAmt (topTrap);
      addTrapSourceToGraph (topTrap);
   }
}

/// TrapSource::getUserNode finds trap based on user selection
EventNode* TrapSource::getUserNode ()
{
   if ( ! m_topNode ) 
   {
      m_topNode = m_eventTree->findNode (m_resName, m_age, m_trapId, -9999, -9999);
   }
   
   return m_topNode;
}

/// TrapSource::setTrapTotalAmt reads the total amount of HC for a trap from the 
/// trap io tbl, which is used as the total amount to be divided up amongst source rocks
void TrapSource::setTrapTotalAmt (EventNode *trap)
{ 
   // get component mass from Trap IO Table
   int tableSize = m_trapIoTbl->size();
   Record *rec;
  
   for ( int i=0; i < tableSize; ++i )
   {
      rec = m_trapIoTbl->getRecord (i);
 
      if ( getReservoirName (rec) == m_resName && 
           getAge (rec) == m_age && 
           getTrapID (rec) == m_trapId ) 
      {         
         trap->setTotalAmt (m_eventTree->getComponentValue (rec));
         break;
      }
   }
}

/// TrapSource::addTrapSourceToGraph adds a line to the source graph for every source rock
/// the X value is the age of the trap and the Y value is the source rock percent of the overall trap amount
void TrapSource::addTrapSourceToGraph (const EventNode *trap)
{
   // for the user-selected trap, loop its source contributions, 
   // working out the contribution value based on its percentage of the overall trap mass
   const SourcePercentContributions &srcContribs = trap->getSourceContributions ();
   const_SourcePercentContributions_IT contribIt, endContrib = srcContribs.end();
   
   for ( contribIt = srcContribs.begin(); contribIt != endContrib; ++contribIt )
   {
      addToLineGroup (contribIt->first, m_negativeAge, (contribIt->second).percent * trap->totalAmt());
   }
}

/// TrapSource::addToLineGroup adds a line (if it doesn't already exist) and adds x,y coordinates to the line
void TrapSource::addToLineGroup (const string& lineName, double x, double y)
{
   // init line points memory if necessary
   LineGroupIT lineIt = m_lineGroup->find (lineName);
   if ( lineIt == m_lineGroup->end () )
   { 
      lineIt = addLineGroup (m_lineGroup, lineName, m_eventTree->tableSize());
   }

   // add point to line
   (lineIt->second).add (x, y);
}

/// TrapSource::addLineGroup adds a new line to a list of lines for the source graph. The name of the line
/// will be a source rock origin
LineGroupIT TrapSource::addLineGroup (LineGroup* lineGroup, const string &name, int size)
{
   // make a new line for the graph
   LineData newLine (size);

   // get age of source layer 
   double age = getSourceAge (name);
   
   // create legend type
   LegendType lt (name, age);
   
   // create pair of legend type and line data 
   pair <LegendType, LineData> newPair (lt, newLine);
   
   // add new legend type to group map and return iterator to it
   return (lineGroup->insert (newPair)).first;
}

double TrapSource::getSourceAge (const string& srcName)
{
   double order = -1;
   int size = m_stratIoTbl->size();
   Record *rec;
  
   for ( int i=0; i < size; ++i )
   {
      rec = m_stratIoTbl->getRecord (i);
      
      // if have found the source rock, get its deposition age
      if ( srcName == getLayerName (rec) )
      {
         order = getDepoAge (rec);
         break;
      }
   }
   
   return order;
}
