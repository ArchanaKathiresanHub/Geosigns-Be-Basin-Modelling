#include <qapplication.h>

#include "traptracking.h"
#include "simplesmartpointer.h"
#include "eventtree.h"
#include "includestreams.h"

// function prototypes
void showUsage (void);

/// Function main creates a QApplication and 
/// a TrapTracking form and then passes the command line arguments into the form
/// the exec command startes the application
int main( int argc, char ** argv )
{   
   // check if user asked for usage only
   if ( (argc == 2) && (strcmp (argv[1], "-h") == 0) ) 
   {
      showUsage ();
      return 0;
   }
   
   // create application
   QApplication a( argc, argv );
    
   // create main form
   TrapTrackingForm f;
         
   f.executeCommandLineArgs (argc, argv);

   // show form
   f.show();
  
   // set up signals and slots for main form
   a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
   // start application
   return a.exec();
}

void showUsage (void)
{
   cout << endl << "Usage:" << endl 
         << endl << "TrapTracking"
         << endl << "-p <Project Filename>"
         << endl << "-r <Reservoir Name>"
         << endl << "-a <Youngest Age>" 
         << endl << "-t <Trap Id>"
         << endl << endl;
}

