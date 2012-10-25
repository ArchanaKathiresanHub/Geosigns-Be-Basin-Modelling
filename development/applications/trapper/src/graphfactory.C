/// Class GraphFactory is a singleton class that holds a list of graph objects
/// It provides create functions for each type of class so controls the creation 
/// and addition of each new graph

#include "currentgraphs.h"
#include "graphfactory.h"
#include "graphobjectslist.h"
#include "rightaxisgraphform.h"
#include "sourcegraphform.h"
#include "componentgraphform.h"

//
// PUBLIC METHODS
//
GraphFactory* GraphFactory::getInstance ()
{
   static GraphFactory graphFactory;
   return &graphFactory;
}
   
RightAxisGraphForm* GraphFactory::createPropertyGraph ()
{
   RightAxisGraphForm* graph = new RightAxisGraphForm ();
   m_graphList->addGraph ("PropertyGraph", graph);
   m_currGraphs->setPropertyGraph (graph);
   return graph;
}

ComponentGraphForm* GraphFactory::createComponentGraph ()
{
   //InteractiveGraphForm *graph = new InteractiveGraphForm ();
   ComponentGraphForm *graph = new ComponentGraphForm ();
   m_graphList->addGraph ("ComponentGraph", graph);
   m_currGraphs->setComponentGraph (graph);
   return graph;
}

SourceGraphForm* GraphFactory::createSourceGraph ()
{
   SourceGraphForm *graph = new SourceGraphForm ();
   m_graphList->addGraph ("SourceGraph", graph);
   m_currGraphs->setSourceGraph (graph);
   return graph;
}
   
void GraphFactory::removeGraph (GraphForm *graph)
{
   m_graphList->removeGraph (graph);
   
   // reset pointers to all current graphs as do not know what type of graph "graph" is!
   m_currGraphs->setPropertyGraph (m_graphList->getLastGraph ("PropertyGraph"));
   m_currGraphs->setComponentGraph (m_graphList->getLastGraph ("ComponentGraph"));
   m_currGraphs->setSourceGraph (m_graphList->getLastGraph ("SourceGraph"));
}

/*
void GraphFactory::removePropertyGraph (RightAxisGraphForm *graph)
{
   m_graphList->removeGraph ("PropertyGraph", graph);
   m_currGraphs->setPropertyGraph (m_graphList->getLastGraph ("PropertyGraph"));
}

void GraphFactory::removeComponentGraph (InteractiveGraphForm *graph)
{
   m_graphList->removeGraph ("ComponentGraph", graph);
   m_currGraphs->setComponentGraph (m_graphList->getLastGraph ("ComponentGraph"));
}

void GraphFactory::removeSourceGraph (SourceGraphForm *graph)
{
   m_graphList->removeGraph ("SourceGraph", graph);
   m_currGraphs->setSourceGraph (m_graphList->getLastGraph ("SourceGraph"));
}
*/
   
//
// PRIVATE METHODS
//
GraphFactory::GraphFactory ()
{
   m_currGraphs = CurrentGraphs::getInstance ();
   m_graphList = GraphObjectsList::getInstance ();
}
