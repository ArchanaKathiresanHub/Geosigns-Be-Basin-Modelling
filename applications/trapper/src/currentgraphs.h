/// Class CurrentGraph holds a pointer to each type of graph
/// that is currently the most recently opened graph of its type
/// This is necessary because the most recently opened graph of each type
/// is the active graph of that type so it is the graph to be updated when
/// a new trap is selected

#ifndef __currentgraphs__
#define __currentgraphs__

#include "graphform.h"

class CurrentGraphs
{
public:
    static CurrentGraphs* getInstance () 
    {
       static CurrentGraphs currGraphs;
       return &currGraphs;
    }
     
    void setPropertyGraph (GraphForm *g) { m_propertyGraph = g; }
    void setComponentGraph (GraphForm *g) { m_componentGraph = g; }
    void setSourceGraph (GraphForm *g) { m_sourceGraph = g; }
   
   GraphForm* getPropertyGraph () { return m_propertyGraph; }
   GraphForm* getComponentGraph () { return m_componentGraph; }
   GraphForm* getSourceGraph () { return m_sourceGraph; }
      
private:
   CurrentGraphs () : m_propertyGraph(0), m_componentGraph(0), m_sourceGraph(0) {}
   
   GraphForm *m_propertyGraph;
   GraphForm *m_componentGraph;
   GraphForm *m_sourceGraph;
};

#endif
