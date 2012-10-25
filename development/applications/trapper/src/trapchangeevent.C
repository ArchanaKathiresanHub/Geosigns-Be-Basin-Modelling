/// Class TrapChangeEvent inherits from QEvent and is used for
/// an external trap change request from the 4D Viewer
/// It holds a list of arguments that are read from the stdin and filled
/// with a PipeMessage object which sends the event to the TrapTracking form

#include "trapchangeevent.h"
#include "includestreams.h"

TrapChangeEvent::TrapChangeEvent (char *msg)
   : QEvent (static_cast <Type> (TrapChangeEventType)), 
   m_argv (createArgs (msg))
{}
 
/// TrapChangeEvent::createArgs takes a message from the stdin and builds
/// an array of arguments which will be passed to TrapTracking form and delt with
/// as if they have been passed in through the command line
char** TrapChangeEvent::createArgs (char *msg)
{
   string msgStr (msg);
   char **newArgs = new char* [6]; // should be num of  ArgDelimiter + 1
   int newArgc = 0;  
   
   // get first argument
   string sub = getSubString (msgStr, ArgDelimiter);
   
   // loop while '/' exist in string
   while ( sub.size() > 0 )
   {
      // issolate argument
      newArgs[newArgc] = new char [sub.size()];
      strcpy (newArgs[newArgc], sub.c_str());
      ++newArgc;
      
      // get next argument
      msgStr = msgStr.substr (sub.size()+1);
      sub = getSubString (msgStr, ArgDelimiter);
   }
    
   // get last arg
   newArgs[newArgc] = new char [msgStr.size()];
   strcpy (newArgs[newArgc], msgStr.c_str());
      
   m_argc = ++newArgc;
   return newArgs;
}

string TrapChangeEvent::getSubString (const string& str, char c)
{
   int index = str.find (c);
   return ( index > -1 && index < (int)str.size()) ? str.substr (0, index) : string("");
}
      
void TrapChangeEvent::deleteArgs ()
{
   for ( int i=0; i < m_argc; ++i )
   {
      delete [] m_argv[i];
   }
   
   delete [] m_argv;
}

