/****************************************************************************
** $Id: qt/main.cpp   3.3.3   edited May 27 2003 $
**
** Copyright ( C ) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <string.h>
#include <signal.h>
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include <qapplication.h>
#include <qmessagebox.h>
#include <qstring.h>

#include "bpaclient.h"
#include "projectview.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

QApplication * app = 0;

bool cbVerbose = false;
bool cbTiming = false;
bool cbTesting = false;

static void sigpipe_handler(int signum)
{
   cerr << "Broken Server Connection, trying to reconnect\n";
   /*
   QMessageBox::information (BPAClient::instance(), ("Broken Server Connection"),
	 ("Connection to BPA Server was closed prematurely\n"
	  "Please reconnect (File->Refresh) and resubmit your command"),
	 QMessageBox::Ok);
    */
}

void myMessageOutput( QtMsgType type, const char *msg )
{
   bool suppressed = false;
   switch ( type ) {
      case QtDebugMsg:
	 fprintf( stderr, "Debug: %s\n", msg );
	 break;
      case QtWarningMsg:
	 if (strstr (msg, "Mutex lock failure: Resource deadlock avoided") != 0)
	 {
	    suppressed = true;
	    break;
	 }
	 if (strstr (msg, "QThread object destroyed while thread is still running") != 0)
	 {
	    suppressed = true;
	    break;
	 }

	 fprintf( stderr, "Warning: %s\n", msg );
	 break;
      case QtFatalMsg:
	 fprintf( stderr, "Fatal: %s\n", msg );
	 abort();                    // deliberately core dump
   }
   if (suppressed)
   {
      // cerr << "Suppressed warning: " << msg << endl;
   }
}



int main( int argc, char ** argv )
{
   QApplication::setColorSpec (QApplication::ManyColor);
   QApplication a (argc, argv, true);

   app = &a;

   // Handle the broken pipe signal
   struct sigaction sa;

   sa.sa_handler = sigpipe_handler;
   sigemptyset (&sa.sa_mask);
   sa.sa_flags = 0;             /* Restart functions if
                                   interrupted by handler */
   if (sigaction (SIGPIPE, &sa, NULL) == -1)
   {
      cerr << "Could not install a signal handler for broken pipe signals" << endl;
   }

   qInstallMsgHandler (myMessageOutput);

   int i;
   char *projectFileName = 0;

   for (i = 1; i < argc; i++)
   {
      int arglen = strlen (argv[i]);
      if (strncmp (argv[i], "-", 1) != 0)
      {
         projectFileName = argv[i];
      }
      else if (strncmp (argv[i], "-verbose", MAX (arglen, 2)) == 0)
      {
         cbVerbose = true;
      }
      else if (strncmp (argv[i], "-timing", MAX (arglen, 3)) == 0)
      {
         cbTiming = true;
      }
      else if (strncmp (argv[i], "-testing", MAX (arglen, 3)) == 0)
      {
         cbTesting = true;
      }
      else
      {
         cerr << "Usage: " << argv[0] << " [projectfile]" << endl;
         return -1;
      }
   }

   a.connect (&a, SIGNAL (lastWindowClosed ()), &a, SLOT (quit ()));

   ProjectView * pv = new ProjectView;
   pv->show ();

   if (projectFileName)
   {
      pv->loadProject (projectFileName);
   }

   BPAClient *cw = BPAClient::instance ();
   cw->addProjectView (pv);
   cw->updateRecentProjects ();

   int result = a.exec ();

   delete cw;
   return result;
}
