///
///
///


#include <qapplication.h>
#include "includestreams.h"
#include "JobLauncherUI.hpp"
#include "GlobalData.h"

#include <signal.h>           // sigaction

///////////////////////////////////////////////////////////////////////
// See QThread for more information here
//
// this is a dummy definition and will start the thread in the backgrounsd
// routine activated when INTR <Control/C> signal has been detected
static void mySignalHandler( int signum )
{
#ifdef DEBUG 
  cout << endl 
       << endl 
       << "mySignalHandler: signal detected = " << signum << endl 
       << endl;
#endif
}


// shows how to use gempil
void showUsage ( char * progname )
{
   cout << endl 
        << "Usage:" << endl 
        << progname << " [-h] <Job Filename>" 
        << endl 
        << endl;
}


///////////////////////////////////////////////////////////////////////
///
/// Function main creates a QApplication and 
/// a form and then passes the command line arguments into the form
/// the exec command startes the application
int main( int argc, char ** argv )
{
#ifdef DEBUG
  cout << "Starting " << argv[0] << endl;
#endif

  // check if user asked for usage only
  if ( (argc >= 2) && strcmp(argv[1], "-h") == 0 ) 
  {
    showUsage ( argv[0] );
    return 0;
  }

  // assign path to globalData
  QString str = argv[0];
  GlobalData::SetPath( str.section( '/', 0, -2 ) );

  //
  // this needs to be done for all of gempil once on startup
  //
  // Turn Off the SIGINT interupt handling.
  struct sigaction new_action;
  new_action.sa_handler = mySignalHandler;
  if ( sigemptyset( &new_action.sa_mask ) == -1 )
    {
      cout << "Gempil - could not define error handler, continuing." << endl;
    }
  else
    {
      // define handler for signal, don't care about the old handler
      new_action.sa_flags = 0;
      sigaction( SIGINT, &new_action, NULL );

#ifdef DEBUG
      cout << "Signal handler defined " << endl;
#endif
    }

  // create application
  QApplication a( argc, argv );

  // fix font size so the application does not use the Qt default
  QFont font("Sans Serif", 10, QFont::Normal, false );
  font.setStyleStrategy( QFont::PreferAntialias );
  a.setFont( font );
   
  // create main form
  JobLauncherUI f;
         
  // show form
  f.show();
  
  // set up signals and slots for main form
  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
  // start application
  return a.exec();
}
