#ifndef __commandthread__
#define __commandthread__

#include <stdlib.h>

/// Command Thread handles a separate thread for detecting
/// external commands to trapper
/// It contains basic thread functionality: start/stop/run
/// When it receives a command it sends it to a 
/// TrapTrackingForm in the for of a TrapChangeEvent

#include <qthread.h>
#include "pipemessage.h"

class TrapTrackingForm;

class CommandThread : public QThread
{
public:
   CommandThread () : QThread (), m_message (0) {}
   virtual ~CommandThread (); 
   
   void init (TrapTrackingForm *tt_form);
   virtual void run ();
   void stopRun ();
   
private:
   char *m_message;
   TrapTrackingForm *m_trapTracking;
};

#endif
