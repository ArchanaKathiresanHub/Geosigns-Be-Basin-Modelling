/// Class TrapChangeEvent inherits from QEvent and is used for
/// an external trap change request from the 4D Viewer
/// It holds a list of arguments that are read from the stdin and filled
/// with a PipeMessage object which sends the event to the TrapTracking form

#ifndef __trapchangeevent__
#define __trapchangeevent__

#include <qevent.h>

#include <string>
using namespace std;

// 
// Create a unique Event type for a Trap Change request
// from an external programme, such as 4D Viewer
//

class TrapChangeEvent : public QEvent
{
public:
   TrapChangeEvent (char *msg);
   ~TrapChangeEvent () { deleteArgs (); }
   
   char** getArgs () { return m_argv; }
   int numArgs () { return m_argc; }
   
   // create unique Type ID for trap change event
   static const int TrapChangeEventType = QEvent::User+1;
   static const char ArgDelimiter = '#';
   
private:
   char** createArgs (char *msg);
   void deleteArgs ();
   string getSubString (const string& str, char c);
   
   char **m_argv;
   int m_argc;
};

#endif
