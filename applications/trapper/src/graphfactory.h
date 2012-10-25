/// Class GraphFactory is a singleton class that holds a list of graph objects
/// It provides create functions for each type of class so controls the creation 
/// and addition of each new graph

#ifndef __graphfactory__
#define __graphfactory__

class CurrentGraphs;
class GraphForm;
class RightAxisGraphForm;
class InteractiveGraphForm;
class SourceGraphForm;
class GraphObjectsList;
class ComponentGraphForm;

class GraphFactory
{
public:
   static GraphFactory* getInstance ();
   
    RightAxisGraphForm* createPropertyGraph ();
    ComponentGraphForm* createComponentGraph ();
    SourceGraphForm* createSourceGraph ();
    void removeGraph (GraphForm *graph);
//    void removePropertyGraph (RightAxisGraphForm *graph);
//    void removeComponentGraph (InteractiveGraphForm *graph);
//    void removeSourceGraph (SourceGraphForm *graph);
   
private:
   GraphFactory ();
   
   CurrentGraphs *m_currGraphs;
   GraphObjectsList *m_graphList;
   
};

#endif
