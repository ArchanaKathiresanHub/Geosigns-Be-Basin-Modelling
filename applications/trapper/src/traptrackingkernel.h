/// Class TrapTrackingKernel initiates all functionaliy requested by the user
/// through the user interface

#ifndef __traptrackingkernel__
#define __traptrackingkernel__

//
// Main class to control functionality of Trap Tracking
//

#include "persistraps.h"
#include "createuserrequest.h"

class TrapTrackingForm;
class DataReader;
class GraphForm;
class SourceGraphForm;
class RightAxisGraphForm;
class InteractiveGraphForm;
class CreateUserRequest;
class GraphFactory;
class CurrentGraphs;
class UserSettings;

class TrapTrackingKernel
{
public:
   // ctor / dtor
   TrapTrackingKernel ();
   
   // public methods
   void setGui (TrapTrackingForm *gui);
   void openDatabase (const char* projectName);
   string displayGraph (GraphForm *graph, const string title);
   string generatePersisTraps ();
   void clearPersisTraps ();
   void convertUstoSpace (string &fileResName);
   string readGraphData (DataReader *reader);
   const TrapInfoHistory& getTrapHistory () { return m_persisTraps.getPersistentTraps (); }
   string createPropertyGraph ();
   string createComponentsGraph ();
   string createSourceGraph ();
   
   void setupPropertyGraphProperties (RightAxisGraphForm *graph);
   void setupComponnetGraphProperties (InteractiveGraphForm *graph);
   void setupSourceGraphProperties (SourceGraphForm *graph);
   
   void setupPropertyGraphRanges (RightAxisGraphForm *graph);
   void setupComponentGraphRanges (InteractiveGraphForm *graph);
   void setupSourceGraphRanges (SourceGraphForm *graph);
   
   string refreshPropertyGraph();
   string refreshComponentGraph();
   string refreshSourceGraph();
   
   const UserSettings* currentUserSettings ();  
   
   bool checkProjectVersion ();
   
   int getPersisId (const string& resName, const double age, const int transId) const;
   int getTransId (const string& resName, const double age, const int persisId) const; 
         
private:
   // private methods
   string setupUserRequest(CreateUserRequest *useReq);
   
   // private variables
   TrapTrackingForm *m_gui;
   PersisTraps m_persisTraps;
   CreateUserRequest m_userRequest;
   GraphFactory *m_graphFactory;
   CurrentGraphs *m_currGraphs;
};

#endif
