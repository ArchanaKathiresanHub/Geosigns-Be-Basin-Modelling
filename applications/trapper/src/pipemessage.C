/// Class PipeMessage monitors input of a message
/// from an external executable

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include "includestreams.h"
#include "pipemessage.h"

int PipeMessage::State = PipeMessage::Stop;

/// PipeMessage::getMessage checks for messages collected
/// with the Select statement until the States does not equal Run or 
/// a message has been received
bool PipeMessage::getMessage (char *msg)
{
   bool checkForMsg = true;

   while ( State == Run && checkForMsg )
   {
      // get message waiting status on STDIN
      int stdin_select = trySelect (STDIN_FILENO);
   
      // switch message status
      switch ( stdin_select )
      {
      case NoMessage:
         {
            break;
         }   
      case MessageWaiting:
         {
            // input message waiting, so read
            msg[0] = '\0';
            scanf ("%s", msg);
            checkForMsg = false;
            
            // check for end of file
            if ( (strlen (msg) == 0) )
            {          
               State = Stop;
            }
            break;
         } 
      case MessageError:
      default:
         {
            // if error, then exit with no message
            State = Stop;
            break;
         }
      } // end switch    
   }
   
   return State == Run;
}

/// PipeMessage::trySelect waits for a message from stdin
/// if a message received it sends a MessageWaiting flag back to 
/// the calling function which can then read the stdin with a scanf
/// to collect the message
int PipeMessage::trySelect (int fd)
{
   // create fd set
   fd_set fdset;
 
   // empty fd set
   FD_ZERO (&fdset);

   // add file descriptor to fd set
   FD_SET (fd, &fdset);
  
   // set timer to zero
   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = 0;

   // call select 
   int result = select (fd+1, &fdset, NULL, NULL, &tv);
  
   // check for error
   if ( result < 0 ) return MessageError;

   // check if fd still exists in set
   // Changed format to compensate for compiler error
   if ( FD_ISSET (fd, &fdset) ) 
        return MessageWaiting;
   else
        return NoMessage;
}

char* PipeMessage::initMsg ()
{
   return new char [MessageLength];
}

void PipeMessage::deleteMsg (char *msg)
{
   delete [] msg;
}

void PipeMessage::setStateToStop ()
{
   State = Stop;
}

void PipeMessage::setStateToRun ()
{
   State = Run;
}

bool PipeMessage::pipeOpen () 
{ 
   return State == Run; 
}
