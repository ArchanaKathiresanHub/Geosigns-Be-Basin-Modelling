/// Class PipeMessage monitors input of a message
/// from an external executable

#ifndef __pipemessage__
#define __pipemessage__

class PipeMessage
{
public:
   static bool getMessage (char *msg);
   static int trySelect (int fd);
   static char* initMsg ();
   static void deleteMsg (char *msg);
   static void setStateToStop ();
   static void setStateToRun ();
   static bool pipeOpen ();
   
   // static variables - fixed length okay as always  
   // reading same message format
   static const int MessageLength = 2000;
   static const int MessageError = -1;
   static const int MessageWaiting = 1;
   static const int NoMessage = 0;
   
private:
   enum State_Type { Stop=0, Run };
   static int State;
};

#endif
