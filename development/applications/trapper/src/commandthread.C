/// Command Thread handles a separate thread for detecting
/// external commands to trapper
/// It contains basic thread functionality: start/stop/run
/// When it receives a command it sends it to a 
/// TrapTrackingForm in the for of a TrapChangeEvent

#include "commandthread.h"
#include "pipemessage.h"
#include "includestreams.h"
#include "traptracking.h"
#include "trapchangeevent.h"

CommandThread::~CommandThread ()
{ 
   if ( m_message ) 
   {
      PipeMessage::deleteMsg (m_message); 
   }
}

/// The Init function takes in a TrapTrackingForm to point to,
/// creates a pipe message type and sets the thread's run states to init
void CommandThread::init (TrapTrackingForm *tt_form)
{
   m_trapTracking = tt_form;
   m_message = PipeMessage::initMsg ();
   PipeMessage::setStateToRun();
}

/// Run loops until there are no more messages coming
/// from the pipe message object. Each time a message is
/// collect, it creates a TrapChangeEvent, passing it the message
/// and then sends the event to the TrapTrackingForm object
void CommandThread::run()
{
   // loop here until trap tracking closed by calling executable
   // or interrupted by traptracking close
   while ( PipeMessage::getMessage (m_message) )
   {
      TrapChangeEvent *trapChange = new TrapChangeEvent (m_message);
      postEvent ( reinterpret_cast <QObject *>(m_trapTracking), trapChange);
   }
}

void CommandThread::stopRun ()
{
   PipeMessage::setStateToStop ();
}

