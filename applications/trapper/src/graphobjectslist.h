/// Class GraphObjectList uses an stl::map to hold
/// GraphObjects
/// It contains usual list minipulation functions:
/// add, remove, count etc.

#ifndef __graphobjectslist__
#define __graphobjectslist__

#include <string>
#include <map>
using namespace std;

class GraphForm;

class GraphObjectsList
{
public:
   static GraphObjectsList* getInstance ();
   void removeGraph (const char* key, GraphForm *graph);
   void removeGraph (GraphForm *graph);
   void addGraph (const char* key, GraphForm *graph);
   int graphCount () const { return m_graphList.size(); }
   void deleteAllGraphs ();
   GraphForm* getLastGraph (const char* key);

private:
   // ctor / dtor (private as singleton class )
   GraphObjectsList () {}
   
   // forbidden assignment
   GraphObjectsList& operator= (const GraphObjectsList& rhs);
   
   // private members
   typedef multimap <string, GraphForm*> GraphList;
   typedef GraphList::iterator GraphListIT;
   GraphList m_graphList;
};

#endif
