/*********************************************************************
*                                                                     
* Package:        GEneric MPI Launcher (gempil)
*
* Dependencies:   runs on Linux (64-bit), Solaris (32-bit), and
*                 Irix (32-bit) platforms; 
*                 Qt windowing system; 
*
* Usage Notes:    See Administrators help 
*
*********************************************************************/

/// Command Thread handles a separate thread for detecting
/// 
/// 
/// When it receives a command it sends it to a 
/// 

// #define DEBUG_GEMPIL
#include "includestreams.h"  // platform type definitions

// project include files
#include <JobLauncherController.h>
#include <JobLauncherUI.hpp>
#include <SubmitStatusUI.hpp>
#include <LocalCommandUI.hpp>
#include <GempisThread.h>
#include <ResourceLimits.h>

// Qt include files
#include <qapplication.h> 
#include <qcombobox.h>
#include <qlineedit.h>

#include <qmessagebox.h> 
#include <qpopupmenu.h> 
#include <qpushbutton.h> 
#include <qstring.h>
#include <qstringlist.h>
#include <qtextedit.h> 
#include <qtimer.h> 
#include <qvaluelist.h> 
#include <qfile.h>
#include <qtextstream.h>
#include <qaction.h>

#include <string>
#include <stdio.h>        // putenv
#include <unistd.h>       // sleep
#include <sys/sysinfo.h>  // sysinfo and get_nprocs
#include <errno.h>        // errno


#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)


///////////////////////////////////////////////////////////////////////
//
// c'tor
//
// CALLED FROM JobLauncherUI.ui.h
//
JobLauncherController::JobLauncherController( JobLauncherUI * select ) :
  m_autoClose( false ),
  m_jobFileRead( false ),
  m_memoryRequiredDefined( false ),
  m_resourceConfigLoaded( false ),
  m_resourceLimitsDefined( false ),
  m_submitButtonActive( true ),
  m_launchGempir( false ),
  m_commandIndex( 0 ),
  m_jobFileName( "" ),
  m_requirementsCommand( "" ),
  m_memoryRequired( "" ),
  m_projectName( "" ),
  m_resourceName( "" ),
  m_processorsTotal( "0" ),
  m_processorsMemoryper( "" ),
  m_processorsSelected( "" ),
  m_envstring( "" ),
  m_projectLog( "" ),
  m_memoryAllocated( "" ),
  m_resourceList(),
  m_select( select ),
  m_resourceLimits( 0 )
{
#ifdef DEBUG_GEMPIL
   cout << this << " JobLauncherController::JobLauncherController()" << endl;
#endif

   // reset result and background color
   long i;
   for (i = 0; i < MAX_COMMANDS; i++)
   {
      m_commandResult[i] = 0;
      m_commandType[i] = "";
      m_description[i] = "";
      m_commandString[i] = "";
   }

  // delete resources.log file if it exists
  QFile rf( "resources.log" );
  if ( rf.exists() )
    {
      rf.remove();
    }

  // commands are always displayed
  commandsEnable(true);

   // disable values display for resources
  processorListEnable(false);
  memoryAllocatedEnable( false );

  bool autostart = false;
  bool verbose = false;
  
  QString numProcessors ("");
  QString resource ("");

   // dummy processing to find the job file name first
   for (i = 1; i < qApp->argc (); i++)
   {
      if (strncmp (qApp->argv ()[i], "-autoclose", Max (6, strlen (qApp->argv ()[i]))) == 0)
      {
         m_autoClose = true;
         if (verbose) cerr << "Gempil:: automatic close" << endl;
      }
      else if (strncmp (qApp->argv ()[i], "-autostart", Max (6, strlen (qApp->argv ()[i]))) == 0)
      {
         autostart = true;
         if (verbose) cerr << "Gempil:: automatic start" << endl;
      }
      else if (strncmp (qApp->argv ()[i], "-processors", Max (2, strlen (qApp->argv ()[i]))) == 0)
      {
         numProcessors = qApp->argv ()[++i];
         if (verbose) cerr << "Gempil:: # processors = " << numProcessors << endl;
      }
      else if (strncmp (qApp->argv ()[i], "-resource", Max (2, strlen (qApp->argv ()[i]))) == 0)
      {
         resource = qApp->argv ()[++i];
         if (verbose) cerr << "Gempil:: resource = " << resource << endl;
      }
      else if (strncmp (qApp->argv ()[i], "-verbose", Max (2, strlen (qApp->argv ()[i]))) == 0)
      {
         cerr << "Gempil:: verbose" << endl;
         verbose = true;
      }
      else if (strncmp (qApp->argv ()[i], "-", 1) == 0)
      {
         cerr << "Gempil:: Unknown option: " << qApp->argv ()[i] << ", Usage: " << qApp->argv ()[0] 
              << "[-processors #processors] [-resource resourcename] [-autostart] [commandfile]" << endl;
      }
      else
      {
         m_jobFileName = qApp->argv ()[i];
         if (verbose) cerr << "Gempil:: job file name = " << m_jobFileName << endl;
      }
   }

   // start time to update the display every second
   m_appTimer = new AppTimer( this, SLOT(childLauncher()) );
   m_appTimer->Set( 1000 );
   m_appTimer->Start();

   if (resource == "")
     {
       resource = "LOCAL";
     }
   else
   {
     // failure to do this first looses the resource name
     resourceListSelect( "LOCAL" );
     m_memoryRequired = "0";
   }

   ReadResourceConfig();
   ReadJobFile ();

   resourceListSelect( resource );
   processorListSelect( numProcessors );

   DisplayWindow ();

   // define activation signal for resource name combobox select
   connect (m_select->ResourceNamecomboBox, SIGNAL (activated (const QString &)),
            this, SLOT (resourceListSelect (const QString &)));

   // define activation signal for number of processors combobox select
   connect (m_select->ResourceNumbercomboBox, SIGNAL (activated (const QString &)),
            this, SLOT (processorListSelect (const QString &)));

   // define activation signal for submit button
   connect (m_select->SubmitpushButton, SIGNAL (clicked ()), this, SLOT (jobStart ()));

   // define activation signal for close button
   connect (m_select->ClosepushButton, SIGNAL (clicked ()), this, SLOT (terminateApplication ()));

   // define launch Gempir button
   connect (m_select->launchGempirpushButton, SIGNAL (clicked ()), this, SLOT (enableGempir ()));

   if (autostart && m_resourceName != "" && m_processorsSelected != "")
   {
      jobStart();
   }
}

///////////////////////////////////////////////////////////////////////
//
// d'tor
//
JobLauncherController::~JobLauncherController()
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::~JobLauncherController()" << endl;
#endif

  // remove old resource limits, if any, and create new one
  if ( m_resourceLimits != 0 )
    {
      delete m_resourceLimits;
    }

  delete m_appTimer;
}


///////////////////////////////////////////////////////////////////////
//
// called when the Close button has been pushed
void JobLauncherController::terminateApplication( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::terminateApplication()" << endl;
#endif

  m_select->close();
  // 
  // !!!!!!!!!!!!!!!!!
  // make sure to put nothing after this line
}


///////////////////////////////////////////////////////////////////////
//
// 
//
void JobLauncherController::childLauncher( void )
{
  if ( m_launchGempir == true )
    {
      launchGempir();
      m_launchGempir = false;
    }

  // restart timer
  m_appTimer->Start();
}


///////////////////////////////////////////////////////////////////////
//
// get the estimated amount of memory
//
// Format is 
// MeSsAgE DATA MEMORY xxx
void JobLauncherController::getMemoryRequirements( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::getMemoryRequirements()" << endl;
#endif

  // reset previous values
  m_memoryRequiredDefined = false;
  m_memoryRequired = "?";

  // no command to run then quit
  if ( m_requirementsCommand.isEmpty() )
    {
      return;
    }

  // define the Gempis requirements command 
  GempisThread * reqThread = new GempisThread();
  reqThread->SetCommand( m_requirementsCommand );
  reqThread->SetLogFileName( m_projectLog );

  // calling in this way runs the command in the foreground
  reqThread->run();

  // check the result of the command
  if ( reqThread->GetResult() )
    {
      // update status bar with error messages
      m_select->statusBar()->message( "Error: Failure getting memory requirements.", 3000 );
    }
  else
    {
      // parse string output for memory message
      while ( reqThread->GetOutputAvailable() == true )
        {
          QString str;
          str = reqThread->GetNextOutputLine();

          QString field1, field2, field3;
          field1 = str.section( ' ', 0, 0, QString::SectionSkipEmpty ); 
          field2 = str.section( ' ', 1, 1, QString::SectionSkipEmpty );
          field3 = str.section( ' ', 2, 2, QString::SectionSkipEmpty );

          // Check for memory message 
          if ( field1 == "MeSsAgE" && field2 == "DATA" && field3 == "MEMORY" )
            {
              QString field4;
              field4 = str.section( ' ', 3, 3, QString::SectionSkipEmpty );

              m_memoryRequired = field4;
              break;
            }
        }
      // if memory value is valid set flag
      if ( !m_memoryRequired.isEmpty() && m_memoryRequired != "?" )
        {
          m_memoryRequiredDefined = true;
        }
    }

  // remove local data
  delete reqThread;
}


///////////////////////////////////////////////////////////////////////
//
// called when a sequence of jobs is to be submitted
void JobLauncherController::jobStart( void )
{
#ifdef DEBUG_GEMPIL
   cout << this << " JobLauncherController::jobStart()" << endl;
#endif

   // before starting make sure there is enough memory to run the job
#if 0
   if (m_memoryAllocated.toLong () < m_memoryRequired.toLong ())
   {
      long max = m_processorsTotal.toLong () * m_processorsMemoryper.toLong ();
      QString message;

      if (m_memoryRequired.toLong () > max)
      {
         message = "This resource probably has insufficient memory to run this project.";
      }
      else
      {
         message = "The selected number of processors is probably not sufficient.";
         long procs = (m_memoryRequired.toLong () + m_processorsMemoryper.toLong () - 1) / m_processorsMemoryper.toLong ();
         QString str;

         str.setNum (procs);
         message += "\nIt is recomended to use at least " + str + " processors.";
      }
      message += "\nContinuing could cause problems on these computers. \n\nDo you want to continue?";
      QMessageBox *box = new QMessageBox ("Not enough memory!",
                                          message,
                                          QMessageBox::Warning,
                                          QMessageBox::Yes,
                                          QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
                                          QMessageBox::NoButton);
      int res = box->exec ();

      delete box;
      if (res == QMessageBox::No)
      {
         return;
      }
   }
#endif

   // disable menu item File->Open and File->Exit
   m_select->fileOpenAction->setEnabled (false);
   m_select->fileExitAction->setEnabled (false);

   // clear status bar of any messages
   m_select->statusBar ()->message ("");

   // turn off any submission fields
   resourceListEnable (false);
   processorListEnable (false);
   memoryAllocatedEnable (false);
   enableCloseButton (false);
   enableSubmitButton (false);

   // update cursor
   commandsDisplayCursor ();

   // determine the job type
   if (m_commandType[m_commandIndex] == "LOCAL")
   {
      // build gempis command and pass this over to the SubmitStatus dialog box
      m_lcDialog = new LocalCommandUI ();

      // define activation signal for resource name combobox select
      connect (m_lcDialog, SIGNAL (jobComplete ()), this, SLOT (jobNext ()));

      m_lcDialog->DoLocalCommand (this);
   }
   else
   {
      // build gempis command and pass this over to the SubmitStatus dialog box
      m_ssDialog = new SubmitStatusUI ();

      // define activation signal for resource name combobox select
      connect (m_ssDialog, SIGNAL (jobComplete ()), this, SLOT (jobNext ()));

      // pass the pointer into this class information and  execute the command
      m_ssDialog->DoParallelCommand (this);
   }
}


///////////////////////////////////////////////////////////////////////
//
// called when the next job is a sequence is to be submitted
void JobLauncherController::jobNext( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::jobNext()" << endl;
#endif

#ifdef DODELETE
  // delete the handle from the previous job
  if (  m_commandType[m_commandIndex] == "LOCAL" )
  {
     // we can't delete m_lcDialog as we return back to it after jobNext () finishes
     delete m_lcDialog;
  }
  else
  {
     // only delete the window if it completed successfully
     if ( m_commandResult[ m_commandIndex ] <= 0 )
     {
	// we can't delete m_ssDialog as we return back to it after jobNext () finishes
	// delete m_ssDialog;
     }
  }
#endif

  // update results
  commandsDisplayResult();
  
  // on error exit back to main window
  if ( m_commandResult[ m_commandIndex ] )
    {
      jobInterrupt();
      return;
    }

  // increment command index
  m_commandIndex++;

  // have we completed all the jobs
  if ( m_commandType[m_commandIndex].isEmpty() )
  {
    jobComplete();

    return;
  }

  // start the job
  jobStart();
}

///////////////////////////////////////////////////////////////////////
//
// job is complete
void JobLauncherController::jobComplete( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::jobComplete()" << endl;
#endif

  // enable menu item File->Exit
  m_select->fileExitAction->setEnabled( true );

  // disable everything except close button
  projectInfoEnable( false );
  processorListEnable( false );
  memoryAllocatedEnable( false );
  resourceListEnable( false );
  enableSubmitButton( false );
  enableCloseButton( true );

  // update log file with final results
  LogFile * lf;
  lf = new LogFile( m_projectLog );
  lf->WriteLine( "//////////////////////////////////////////////////////////////" );
  lf->WriteLine( "-PrOjEcT Completed " );
  lf->WriteLine( "-PrOjEcT Results:      " );
  for ( int i=0; i<m_commandIndex; i++ )
    {
      lf->Write( "-PrOjEcT       result: " );
      lf->WriteLine( m_commandResult[ i ] );
    }
  QDateTime dt;
  dt = QDateTime::currentDateTime();
  lf->Write( "-PrOjEcT Completed on: " );
  lf->WriteLine( dt.toString( "ddd MMMM d, yyyy hh:mm:ss" ) );
  delete lf;

  if (m_autoClose)
    {
      terminateApplication();
    }
}

///////////////////////////////////////////////////////////////////////
//
// job has been interupted
void JobLauncherController::jobInterrupt( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::jobInterrupt()" << endl;
#endif

  // enable menu item File->Exit
  m_select->fileExitAction->setEnabled( true );

  resourceListEnable( true );
  processorListEnable( true );
  memoryAllocatedEnable( true );

  // turn buttons back on
  enableSubmitButton( true );
  enableCloseButton( true );
}


///////////////////////////////////////////////////////////////////////
//
// set title bar
void JobLauncherController::setWindowTitle( void )
{
  QString stg;
  stg = "Gempil project ";
  stg += m_projectName;
  m_select->setCaption( stg );
}

///////////////////////////////////////////////////////////////////////
//
//
void JobLauncherController::enableSubmitButton( bool isOn )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::enableSubmitButton(" << isOn << ")" << endl;
#endif
  m_select->SubmitpushButton->setEnabled( isOn );
}

///////////////////////////////////////////////////////////////////////
//
//
void JobLauncherController::setTextSubmitButton()
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::setTextSubmitButton()" << endl;
#endif
  /*
  if ( m_submitButtonActive )
    {
      m_select->SubmitpushButton->setText( "Submit" );
    }
  else
    {
      m_select->SubmitpushButton->setText( "Terminate" );
    }
  */
}

///////////////////////////////////////////////////////////////////////
//
//
void JobLauncherController::enableCloseButton( bool isOn )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::enableCloseButton(" << isOn << ")" << endl;
#endif
  m_select->ClosepushButton->setEnabled( isOn );
}


///////////////////////////////////////////////////////////////////////
//
// project information members
//
// enable display
void JobLauncherController::projectInfoEnable( bool isOn )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::projectInfoEnable(" << isOn << ")" << endl;
#endif

  if ( isOn == true && m_projectName.length() > 0 )
    {
      m_select->ProjectNamelineEdit->setEnabled( true );
    }
  else
    {
      m_select->ProjectNamelineEdit->setEnabled( false );
    }

  // suggested memory size
  if ( isOn == true && m_memoryRequiredDefined == true  )
    {
      m_select->ProjectMemorylineEdit->setEnabled( true );
    }
  else
    {
      m_select->ProjectMemorylineEdit->setEnabled( false );
    }
}

// display of project information
void JobLauncherController::projectInfoDisplay( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::projectInfoDisplay()" << endl;
#endif

  m_select->ProjectNamelineEdit->setText( m_projectName );
  m_select->ProjectMemorylineEdit->setText( m_memoryRequired );
}


///////////////////////////////////////////////////////////////////////
//
// execution commands members
//
// enable display
void JobLauncherController::commandsEnable( bool isOn )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::commandsEnable(" << isOn << ")" << endl;
#endif

  m_select->CommandstextEdit->setEnabled( isOn );
}

// initial display of project information
void JobLauncherController::commandsDisplay( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::commandsDisplay()" << endl;
#endif

  m_select->CommandstextEdit->clear();
  for ( long i=0; m_commandType[i].length(); i++ )
    {
      m_select->CommandstextEdit->insertParagraph( m_commandString[i], -1 );
    }
}

// update background color of the current job
void JobLauncherController::commandsDisplayResult( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::commandsDisplayResult()" << endl;
#endif

  // determine the result
  QColor color;
  QColor goodColor(green);
  QColor badColor(red);
  QColor warningColor(yellow);
  QColor abortColor(magenta);
  if ( m_commandResult[m_commandIndex] == 0 )
    {
      color = goodColor;
    }
  else if ( m_commandResult[m_commandIndex] == 1 )
    {
      color = badColor;
    }
  else if ( m_commandResult[m_commandIndex] == 2 )
    {
      color = warningColor;
      // reset warning to continue
      m_commandResult[m_commandIndex] = 0;
    }
  else
    {
      color = abortColor;
    }

  // update line with result
  m_select->CommandstextEdit->setSelection( m_commandIndex, 0, m_commandIndex+1, 0 );
  m_select->CommandstextEdit->setColor(QColor(black));
  m_select->CommandstextEdit->removeSelection();
  m_select->CommandstextEdit->setParagraphBackgroundColor( m_commandIndex, color );
}

// set cursor to current command
void JobLauncherController::commandsDisplayCursor( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::commandsDisplayCursor()" << endl;
#endif

  // set cursor location
  m_select->CommandstextEdit->setCursorPosition( m_commandIndex, 0 );

  // invert the line
  m_select->CommandstextEdit->setSelection( m_commandIndex, 0, m_commandIndex+1, 0 );
  m_select->CommandstextEdit->setColor(QColor(white));
  m_select->CommandstextEdit->removeSelection();
  m_select->CommandstextEdit->setParagraphBackgroundColor( m_commandIndex, QColor(black) );
}


////////////////////////////////////////////////////////////////////////////
//
// resource list members
//
void JobLauncherController::resourceListEnable( bool isOn )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::resourceListEnable(" << isOn << ")" << endl;
#endif

  // make sure that the resource selection is on
  m_select->ResourceNamecomboBox->setEnabled( isOn );
}

//
void JobLauncherController::resourceListBuild( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::resourceListBuild()" << endl;
#endif

  m_select->ResourceNamecomboBox->clear();
  m_select->ResourceNamecomboBox->insertStringList( m_resourceList );

  // set pointer to current location
  m_select->ResourceNamecomboBox->setCurrentText( m_resourceName );
}

//
void JobLauncherController::resourceListAdjust( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::resourceListAdjust()" << endl;
#endif

  // check job file read and resource defined
  if ( m_jobFileRead == false || m_resourceName.isEmpty() )
    {
      processorListEnable( false );
      memoryAllocatedEnable( false );
      enableSubmitButton( false );

      return;
    }

  // set pointer to current location
  m_select->ResourceNamecomboBox->setCurrentText( m_resourceName );

  // verify that resource limits have been defined
  if ( m_resourceLimitsDefined == false )
    {
      processorListEnable( false );
      memoryAllocatedEnable( false );
      enableSubmitButton( false );

      return;
    }

  // update processors list display
  processorListEnable( true );
  processorListAdjust();

  // update memory available
  memoryAllocatedEnable( true );
  memoryAllocatedUpdate();

  // turn submit button on
  enableSubmitButton( true );
}

// called when the resource name have been selected
//
// name is always accepted and restarted to insure a retry
void JobLauncherController::resourceListSelect( const QString & s )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::resourceListSelect(" << s << ")" << endl;
#endif

  // check that resource name is defined
  if ( s.isEmpty() )
    {
      m_select->statusBar()->message( "Resource name not defined." );

      return;
    }
  m_resourceName = s;

  // reset previous value of processors selected
  m_processorsSelected = "";

  // remove old resource limits, if any, and create new one
  if ( m_resourceLimits != 0 )
    {
      delete m_resourceLimits;
    }
  m_resourceLimits = new ResourceLimits( m_resourceName );

  // update display based on the result
  if ( m_resourceLimits->GetStatus() == ResourceLimits::HAVE_VALUES )
    {
      m_resourceLimitsDefined = true;
    }
  else if ( m_resourceLimits->GetStatus() == ResourceLimits::CREATE_FAILED )
    {
      m_resourceLimitsDefined = false;
      m_select->statusBar()->message( "Failed to load Limit values for resource " + m_resourceName + "." );
    }
  else if ( m_resourceLimits->GetStatus() == ResourceLimits::CREATE_LIMITS )
    {
      m_select->ResourceNamecomboBox->setCurrentText( m_resourceName );
      m_select->statusBar()->message( "Creating resource limits for " + m_resourceName + ", please wait." );

      // disable buttons while running gempis
      resourceListEnable( false );
      processorListEnable( false );
      memoryAllocatedEnable( false );

      // disable submit button
      enableSubmitButton( false );

      // give the user a warning message
      QMessageBox * ptr = new QMessageBox( "Collecting Resource Limits", 
                                           "Gempil needs to collect resource limits for resource " 
                                           + m_resourceName 
                                           + ".\n\n This may take a few minutes and the window will appear to freeze."
                                           + " Please leave the gempil window alone until it has completed."
                                           + "\n\n Press Ok to continue.", 
                                           QMessageBox::Information, 
                                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton  );
      // run gempis
      ptr->exec();
      delete ptr;

      // turn resource list back on
      resourceListEnable( true );

      // create the limit values and check result
      m_resourceLimits->CreateLimits();
      if ( m_resourceLimits->GetStatus() == ResourceLimits::HAVE_VALUES )
        {
          m_resourceLimitsDefined = true;
          m_select->statusBar()->message( "Resource limits for " + m_resourceName + " have been created." );
        }
      else
        {
          m_resourceLimitsDefined = false;
          m_select->statusBar()->message( "Error creating resource limits for " + m_resourceName 
                                          + ". Launch the Resource Manager to find the cause." );
        }
    }

  // set total number of processors
  m_processorsTotal.setNum( m_resourceLimits->GetTotalCores() );
  m_processorsMemoryper.setNum( m_resourceLimits->GetMemoryPerCore() );
#ifdef DEBUG_GEMPIL
  cout << " m_processorsTotal = " << m_processorsTotal << endl;
  cout << " m_processorsMemoryper = " << m_processorsMemoryper << endl;
#endif

  // rebuild processor list
  processorListBuild();

  // adjust resoure name information
  resourceListAdjust();
}


////////////////////////////////////////////////////////////////////
//
// processorList members
//
// enables dropdown menu
void JobLauncherController::processorListEnable( bool isOn )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::processorListEnable(" << isOn << ")" << endl;
#endif

  m_select->ResourceNumbercomboBox->setEnabled( isOn );
}

// build list of selectable number of processors
void JobLauncherController::processorListBuild( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::processorListBuild()" << endl;
#endif

  m_select->ResourceNumbercomboBox->clear();
  for ( long i=1; i<=m_processorsTotal.toLong(); i++ )
    {
      QString str;
      str.setNum( i );
      m_select->ResourceNumbercomboBox->insertItem( str );
    }
}

// called when the list needs to be updated with the correct number of processors
void JobLauncherController::processorListAdjust( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::processorListAdjust()" << endl;
#endif

  // determine the number of processors needed
  QString processorsNeeded;
  if ( m_memoryRequiredDefined == false || m_resourceLimitsDefined == false )
    {
      processorsNeeded = "1";
    }
  else
    {
      // compute the minimum number of processors needed
      long memoryPerProc = m_processorsMemoryper.toLong();

      // if the value is not 0
      if ( memoryPerProc == 0 )
        {
          processorsNeeded = "1";
        }
      else
        {
          // determine how many processors are minimally needed
          long memoryNeeded =  m_memoryRequired.toLong();
          long procsNeeded = ( memoryNeeded + memoryPerProc - 1 ) / memoryPerProc;

          processorsNeeded.setNum( procsNeeded );
        }
    }

  // check that the number of processors is correct and in limits
  if ( m_processorsSelected.isEmpty() || processorsNeeded.toLong() > m_processorsSelected.toLong() )
    {
      m_processorsSelected = processorsNeeded;
    }
  if ( processorsNeeded.toLong() > m_processorsTotal.toLong() )
    {
      m_processorsSelected = m_processorsTotal;
    }

  // set pointer to current location
  m_select->ResourceNumbercomboBox->setCurrentText( m_processorsSelected );

  memoryAllocatedUpdate();
}

// called when the number of processors has been selected
void JobLauncherController::processorListSelect( const QString & s )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::processorListSelect(" << s << ")" << endl;
#endif

  // check for valid value and defined limits
  if ( s.isEmpty() || m_processorsTotal == "0" )
    {
      return;
    }

  // save the number of processors selected
  if ( s.toLong() > m_processorsTotal.toLong() )
    {
      m_processorsSelected = m_processorsTotal;
    }
  else
    {
      m_processorsSelected = s;
    }

  // set pointer to current location
  m_select->ResourceNumbercomboBox->setCurrentText( m_processorsSelected );

  // update the memory allocated
  memoryAllocatedUpdate();
}


///////////////////////////////////////////////////////////////////////
//
// called when ...
void JobLauncherController::memoryAllocatedEnable( bool isOn )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::memoryAllocatedEnable(" << isOn << ")" << endl;
#endif

  m_select->ResourceMemoryAllocatedlineEdit->setEnabled( isOn );
}


///////////////////////////////////////////////////////////////////////
//
// called when ...
void JobLauncherController::memoryAllocatedUpdate( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::memoryAllocatedUpdate()" << endl;
#endif

  long totalMem = 0;
  if ( ! m_processorsSelected.isEmpty() )
    {
      totalMem = m_processorsSelected.toLong();
      totalMem *= m_processorsMemoryper.toLong();
    }
  m_memoryAllocated = QString::number( totalMem );

  m_select->ResourceMemoryAllocatedlineEdit->setText( m_memoryAllocated );

  if ( m_memoryRequiredDefined == true )
    {
      if ( m_memoryAllocated.toLong() >= m_memoryRequired.toLong() )
        {
          m_select->statusBar()->message( "Click Submit to start simulation." );
        }
      else
        {
          m_select->statusBar()->message( "Warning! Not enough memory is available for the simulation." );
        }
    }
}


///////////////////////////////////////////////////////////////////////
//
// (re)load all resources definitions: system and user
void JobLauncherController::ReadResourceConfig( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::ReadResourceConfig()" << endl;
#endif

  // load resource configuration
  ResourceFile * rd = new ResourceFile();
  rd->LoadResources( true );
  m_resourceList = rd->GetResourceList();
  delete rd;

  // check if configuration was successfully loaded
  if ( m_resourceList.size() == 0 )
    {
      m_resourceConfigLoaded = false;
      // paint commands window with message to configure gempis
    }
  else
    {
      m_resourceConfigLoaded = true;
      resourceListBuild();
    }
}

///////////////////////////////////////////////////////////////////////
//
// called when the gempis job file (*.gjf) has been selected
// 
// ALSO CALLED FROM JobLauncherUI.ui.h
//
void JobLauncherController::ReadJobFile( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::ReadJobFile()" << endl;
#endif

  // reset project log file name
  m_projectLog = "";

  // check if gempis is defined, fail if not
  QString gempisFile;
  char * cptr;
  cptr = getenv( "HOME" );
  gempisFile = cptr;
  gempisFile += "/.gempis/definitions_user";
  QFile gf( gempisFile );
  if ( !gf.exists() )
    {
      m_select->statusBar()->message( "Gempis is not been setup. Type 'gempis adm setup' and follow instructions." );
      m_jobFileRead = false;
 
      return;
    }

  if ( m_jobFileName.isEmpty() )
    {
      // update status bar with error messages
      m_select->statusBar()->message( "Open a job file (File -> Open) to continue." );
      return;
    }

  // open the job file of the project
  QFile f( m_jobFileName );
  if ( !f.open( IO_ReadOnly ) )
    {
      // update status bar with any error messages
      m_select->statusBar()->message( "Error: Could not open job file" +  m_jobFileName + ".", 3000 );

      m_jobFileRead = false;
      return;
    }

  // reset command index
  m_commandIndex = 0;

  // parse input file
  QTextStream ts( &f );
  int command_index = 0;
  while ( true )
  {
    QString str;

    str = ts.readLine();
    if ( str.isNull() )
      break;

    QString option;
    option = str.section( ' ', 0, 0, QString::SectionSkipEmpty ); 
    if ( option == NULL )
      {
        option = "";
      }
    QString field1;
    field1 = str.section( '\"', 1, 1, QString::SectionSkipEmpty );
    if ( field1 == NULL )
      {
        field1 = "";
      }
    QString field2;

    field2 = str.section( '\"', 3, 3, QString::SectionSkipEmpty );
    if ( field2 == NULL )
      {
        field2 = "";
      }

    // cout << option << endl;

    if ( option == "#" )
    {
      // do nothing, this is a comment
      continue;
    }
    else if ( option == "PROJECT" )
    {
      m_projectName = field1;

      setWindowTitle();

      // define project log file name
      QString projectFile;
      projectFile = field1;
      projectFile.replace ( QChar(' '), "_" ); 
      projectFile.replace ( QChar('/'), "_" ); 
      m_projectLog = projectFile + ".log";

      // reset log file and put a header it
      LogFile * lf;
      lf = new LogFile( m_projectLog );
      lf->Reset();
      lf->WriteLine( "//////////////////////////////////////////////////////////////" );
      lf->Write( "-PrOjEcT Log File:     " );
      lf->WriteLine( m_projectLog );
      char hostnm[ 128 ];
      gethostname( hostnm, 124 );
      lf->Write( "-PrOjEcT Hostname:     " );
      lf->WriteLine(  hostnm );
      QDateTime dt;
      dt = QDateTime::currentDateTime();
      lf->Write( "-PrOjEcT Started on:   " );
      lf->WriteLine( dt.toString( "ddd MMMM d, yyyy hh:mm:ss" ) );
      delete lf;

      // email log
      lf = new LogFile( "Email.log" );
      lf->Reset();
      delete lf;
    }
    else if ( option == "REQUIREMENTS" )
    {
      m_requirementsCommand = field1;
      if ( ! m_projectLog.isEmpty() )
        {
          // Add entry to log file
          LogFile * lf;
          lf = new LogFile( m_projectLog );
          lf->Write( "-PrOjEcT Requirements: " );
          lf->WriteLine( m_requirementsCommand );
          delete lf;
        }
    }
    else if ( option == "PARALLEL" || option == "LOCAL" )
    {
      if ( field1.isEmpty() )
        {
          continue;
        }
      m_commandType[ command_index ] = option;
      m_commandString[ command_index ] = field1;
      m_description[ command_index ] = field2;
      command_index++;
      if ( ! m_projectLog.isEmpty() )
        {
          // Add entry to log file
          LogFile * lf;
          lf = new LogFile( m_projectLog );
          lf->Write( "-PrOjEcT       entry:  " );
          lf->WriteLine( str );
          delete lf;
        }
    }
    else 
    {
      // update status bar with error messages
      m_select->statusBar()->message( "Unknown line for option " + option + " : " + str + ".", 3000 );
      if ( ! m_projectLog.isEmpty() )
        {
          // Add entry to log file
          LogFile * lf;
          lf = new LogFile( m_projectLog );
          lf->Write( "-PrOjEcT Unknown Entry:" );
          lf->WriteLine( str );
          delete lf;
        }
    }
  }
  f.close();

  // define project name environment variable
  m_envstring = "GEMPIS_PROJECTNAME=" + m_projectName;
  putenv( (char *)(m_envstring.latin1()) );

  // terminate list
  m_commandType[ command_index ] = "";

  // file successfully read
  m_jobFileRead = true;

  getMemoryRequirements();

  // reselect resource (to adjust limits)
  resourceListSelect( m_resourceName );
}


///////////////////////////////////////////////////////////////////////
//
// display of window in all cases
//
// once the project file and resources has been read 
// called when the entire window needs to be updated
//
// ALSO CALLED FROM JobLauncherUI.ui.h
//
void JobLauncherController::DisplayWindow( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::DisplayWindow()" << endl;
#endif

  // project name and commands
  projectInfoEnable( m_jobFileRead );
  projectInfoDisplay();

  // update display
  commandsDisplay();

  resourceListEnable( m_jobFileRead );
  processorListEnable( m_jobFileRead );
  memoryAllocatedEnable( m_jobFileRead );
  memoryAllocatedUpdate();
}


///////////////////////////////////////////////////////////////////////
//
// called when the launch resource manager button has been pushed
//
void JobLauncherController::enableGempir( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::enableGempir()" << endl;
#endif

  m_launchGempir = true;
}

///////////////////////////////////////////////////////////////////////
//
// launches gempir
//
void JobLauncherController::launchGempir( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::launchGempir()" << endl;
#endif

  QString command;
  command = "gempir ";

  // If IBS_VERSION is defined and a valid version number apply it to the command.
  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL && strlen(cptr) > 1 )
    {
      command += "-";
      command += cptr;
    }

  // you can add a resource name if you want here
  command += " ";
  command += m_resourceName;

  // append a redirect for stdout and stderr, and spawn no wait
  command += " >/dev/null 2>&1 <&1 &";

  // start
  system( command );
}


///////////////////////////////////////////////////////////////////////
//
// send an e-mail to helpdesk with project log file
//
// ALSO CALLED FROM JobLauncherUI.ui.h
//
int JobLauncherController::SendEmail( void )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::SendEmail()" << endl;
#endif

  // now tell gempis to send email
  // create gempis job thread
  GempisThread * emailThread = new GempisThread();

  // define the command to email the log
  QString command;
  command = "gempis ";
  // If IBS_VERSION is defined and valid apply it to the gempis command.
  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL && strlen(cptr) > 1 )
    {
      command += "-";
      command += cptr;
    }
  command += " adm ";
  command += " email ";
  if ( m_projectName.isNull() || m_projectName.isEmpty() )
    {
      command = "gempis-default";
    }
  else
    {
      command += m_projectName;
    }
  command += ".log";
  emailThread->SetCommand( command );
  emailThread->SetLogFileName( "Email.log" );

  // calling in this way runs the command in the foreground
  emailThread->run();

  int result = emailThread->GetResult();

  // remove local data
  delete emailThread;

  return result;
}


///////////////////////////////////////////////////////////////////////
//
// parse job file to get information
//
// ALSO CALLED FROM JobLauncherUI.ui.h
//
void JobLauncherController::SetJobFilename ( const char * ptr )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::SetJobFilename()" << endl;
#endif

  m_jobFileName = ptr;
}


///////////////////////////////////////////////////////////////////////
//
QString JobLauncherController::GetProjectName( void ) const
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::GetProjectName()" << endl;
#endif

  return m_projectName;
}

///////////////////////////////////////////////////////////////////////
//
//
QString JobLauncherController::GetProjectLogFileName( void ) const
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::GetProjectLogFileName()" << endl;
#endif

  return m_projectLog;
}

///////////////////////////////////////////////////////////////////////
//
//
long JobLauncherController::GetProjectCommandIndex( void ) const
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::GetProjectCommandIndex()" << endl;
#endif

  return m_commandIndex;
}

///////////////////////////////////////////////////////////////////////
//
//
QString JobLauncherController::GetResourceName( void ) const
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::GetResourceName()" << endl;
#endif

  return m_resourceName;
}

///////////////////////////////////////////////////////////////////////
//
//
QString JobLauncherController::GetNumberProcessors( void ) const
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::GetNumberProcessors()" << endl;
#endif

  return m_processorsSelected;
}

///////////////////////////////////////////////////////////////////////
//
//
QString JobLauncherController::GetCommand( void ) const
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::GetCommand()" << endl;
  cout << "    index is " << m_commandIndex << endl;
  cout << "    command type is " << m_commandType[ m_commandIndex ] << endl;
  cout << "    command description is " << m_description[ m_commandIndex ] << endl;
  cout << "    command is " << m_commandString[ m_commandIndex ] << endl;
#endif

  return m_commandString[ m_commandIndex ];
}

//
///////////////////////////////////////////////////////////////////////
//
QString JobLauncherController::GetCommandType( void ) const
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::GetCommandType()" << endl;
#endif

  return m_commandType[ m_commandIndex ];
}

///////////////////////////////////////////////////////////////////////
//
//
void JobLauncherController::SetCommandResult( long result )
{
#ifdef DEBUG_GEMPIL
  cout << this << " JobLauncherController::SetCommandResult(" << result << ")" << endl;
#endif

  // set value if value is good or error 
  // or if result has not been defined
  m_commandResult[ m_commandIndex ] = result;
}


