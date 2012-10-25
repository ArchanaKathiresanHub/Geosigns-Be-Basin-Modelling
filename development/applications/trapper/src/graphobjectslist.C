/// Class GraphObjectList uses an stl::map to hold
/// GraphObjects
/// It contains usual list minipulation functions:
/// add, remove, count etc.

#include "graphobjectslist.h"
#include "graphform.h"

GraphObjectsList* GraphObjectsList::getInstance ()
{
   static GraphObjectsList graphObjectList;
   return &graphObjectList;
}

/// GraphObjectsList::removeGraph removes all graphs with a certain key
/// a key is a GraphObject type name
void GraphObjectsList::removeGraph (const char* key, GraphForm *graph)
{
   GraphListIT lowerBound = m_graphList.lower_bound (key);
   GraphListIT upperBound = m_graphList.upper_bound (key);
   
   while ( lowerBound != upperBound )
   {
      if ( (lowerBound->second) == graph )
      {
         m_graphList.erase (lowerBound);         
         break;
      }
      
      ++lowerBound;
   }
}

void GraphObjectsList::removeGraph (GraphForm *graph)
{
   GraphListIT graphIt, endGraph = m_graphList.end();
   for ( graphIt = m_graphList.begin(); graphIt != endGraph; ++graphIt )
   {
      if ( graphIt->second == graph )
      {
         m_graphList.erase (graphIt);
         break;
      }
   }
            
}

void GraphObjectsList::addGraph (const char* key, GraphForm *graph)
{
   m_graphList.insert (GraphList::value_type (key, graph));
}

/// GraphObjectsList::getLastGraph returns the most recently added
/// graph of a certain type
GraphForm* GraphObjectsList::getLastGraph (const char* key)
{
   pair <GraphListIT, GraphListIT> rangeIt = m_graphList.equal_range (key);
   
   if ( rangeIt.first != rangeIt.second) 
   {
      // get the last graph entry of type key
      --(rangeIt.second);
      return (rangeIt.second)->second;
   }
   else
   {
      return NULL;
   }
}

void GraphObjectsList::deleteAllGraphs ()
{  
   int graphCount = m_graphList.size();
   int deleteCount = -1;
   
   while ( ++deleteCount < graphCount )
   {
      // deleteing a graph removes it from the list
      // so there will be a new begin each time
      delete (m_graphList.begin()->second);
   }
}
