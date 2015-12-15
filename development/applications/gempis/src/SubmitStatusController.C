///
///
///

#include "includestreams.h"

#include <JobLauncherController.h>
#include <SubmitStatusController.h>
#include <SubmitStatusUI.hpp>
#include <GempisThread.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qtextedit.h> 
#include <qpushbutton.h> 
#include <qapplication.h> 
#include <qdatetime.h> 
#include <qtimer.h> 
#include <qmessagebox.h> 

#include <string>

// process info
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>    // exit()
#include <signal.h>


#define SS_TIMEOUT 1000  // timeout value for refresh


SubmitStatusController::SubmitStatusController( SubmitStatusUI * select ) :
  m_jobCompleteSent( false ),
  m_errorDetected( false ),
  m_warningDetected( false ),
  terminateActive( true ),
  m_childrenActive( true ),
  m_result( 0 ),
  m_lastLineNumber( 0 ),
  m_error( "" ),
  m_skipWarnings( "" ),
  m_skipErrors( "" ),
  m_select( select ), 
  m_controller( 0 )
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::SubmitStatusController()" << endl;
#endif

  // load list or warning and error strings to skip
  char * cptr;
  cptr = getenv( "GEMPIL_IGNORE_ERRORS" );
  if ( cptr != NULL && strlen(cptr) > 1 )
  {
    m_skipErrors = QStringList::split( '|', cptr );
  } 
  cptr = getenv( "GEMPIL_IGNORE_WARNINGS" );
  if ( cptr != NULL && strlen(cptr) > 1 )
  {
    m_skipWarnings = QStringList::split( '|', cptr );
  } 

  // create my copy of the gempis data and job thread
  m_gempisThread = new GempisThread();

  // define activation signal for terminate button
  connect( m_select->TerminatepushButton, SIGNAL( clicked() ), this, SLOT( terminateClicked() ) ) ;

  // start a timer to update the display
  internalSSTimer = new QTimer( this );

  // control taken by active window, show window
  //  m_select->setModal( true );
  m_select->setModal( false );
  m_select->show();
}


SubmitStatusController::~SubmitStatusController()
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::~SubmitStatusController()" << endl;
#endif

  // stop, disconnect and delete the timer
  internalSSTimer->stop();
  disconnect( internalSSTimer, SIGNAL(timeout()), this, SLOT(refreshWindow()) );
  delete internalSSTimer;

  // wait for thread to complete, is probably already done
  m_gempisThread->wait();
  delete m_gempisThread;
}


void SubmitStatusController::StartGempisCommand( JobLauncherController * ptr )
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::StartGempisCommand()" << endl;
#endif

  m_controller = ptr;

  // set title bar
  QString str;
  str = "Submit Status ";
  str += m_controller->GetProjectName() + " ";
  str += m_controller->GetResourceName() + " ";
  m_select->setCaption( str );

  // start building the command to execute
  QString command;
  command = "gempis ";

  // If IBS_VERSION is defined and a valid version number 
  // apply it to the gempis command.
  // IBS_VERSION is defined in the development environment              
  // as "d" or "t"
  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL && strlen(cptr) > 1 )
    {
      command += "-";
      command += cptr;
      command += " ";
    }

  // complete the rest of the command
  command += m_controller->GetResourceName() + " ";
  command += m_controller->GetNumberProcessors() + " ";
  command += m_controller->GetCommand();

  // redirect stdin to null device
  command += "</dev/null";

  // define the Gempis command and log file
  //  str = str.setNum( m_controller->GetProjectCommandIndex() );
  m_gempisThread->SetCommand( command );
  m_gempisThread->SetLogFileName( m_controller->GetProjectLogFileName());

  // start update timer
  internalSSTimer->singleShot( SS_TIMEOUT, this, SLOT(refreshWindow()) );

  // spawn child to execute command
  m_gempisThread->start();
}


void SubmitStatusController::KillChildExecutable( void )
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::KillChildExecutable()" << endl;
#endif

  // do we have an active job running
  if ( m_childrenActive == true )
    {
      // tell the Gempis thread to kill itself
      m_gempisThread->KillThread();
    }
}


bool SubmitStatusController::GetJobCompleteSent()
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::GetJobCompleteSent()" << endl;
#endif

  return m_jobCompleteSent;
}


void SubmitStatusController::SetJobCompleteSent()
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::SetJobCompleteSent()" << endl;
#endif

  m_jobCompleteSent = true;
}


void SubmitStatusController::refreshWindow( void )
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::refreshWindow()" << endl;
#endif

  updateDisplay();

  // process events
  qApp->processEvents();

  // check the child has terminated
  if ( m_gempisThread->finished() )
    {
      m_result = m_gempisThread->GetResult();

      // finish up command processing
      commandComplete();

      // indicate that child process is no longer running
      m_childrenActive = false;
    }
  else
    {
      // restart timer 
      internalSSTimer->singleShot( SS_TIMEOUT, this, SLOT(refreshWindow()) );
    }
}


void SubmitStatusController::commandComplete( void )
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::commandComplete()"<< endl;
#endif

  // if the command was not terminated externally, set the result
  // if not successful
  QString stg;
  stg = "SUCCESS";
  if ( m_result )
    {
      // when an error has occurred:
      // stop the process
      // the user has to acknowledge the error to return to the 
      // main window
      m_controller->SetCommandResult( 1 );
      stg = "FAILURE";

      // set reason to error return value
      QString reason;
      reason = "The application returned an error of ";
      QString dummy;
      reason += dummy.setNum( m_result );
      m_select->ExecutionReasonlineEdit->setText( reason );
    }
  else if ( m_errorDetected || m_warningDetected )
    {
      // when only text has been detected, keep window open
      // but allow the job to proceed further
      m_controller->SetCommandResult( 2 );
      m_select->Continue2NextJob();
      if ( m_errorDetected )
        {
          stg = "ERROR"; 
        }
      else
        {
          stg = "WARNING";
        }
      m_select->ExecutionReasonlineEdit->setText( "Review the window for messages" );
    }
  else
    {
      // when the process has been successful, close 
      // the window and proceed to the next job
      m_controller->SetCommandResult( 0 );
      // disable update timer and close the window
      m_select->close();
      // make sure that nothing else happens after the close, 
      // cause for some strange reason the application DOES 
      // return from the close
      return;
    }

  // 
  // if here then there was a possible error
  //

  // change text to close and indicate that terminate button is not active
  m_select->ExecutionResultlineEdit->setText( stg );
  m_select->TerminatepushButton->setText( "Close" );

  if ( m_result || m_errorDetected || m_warningDetected )
    {
      terminateActive = false;
    }
  else
    {
      disconnect( internalSSTimer, SIGNAL(timeout()), this, SLOT(refreshWindow()) );
      m_select->close();
    }
}


void SubmitStatusController::terminateClicked( void ) 
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::terminateClicked()" << endl;
#endif

  // if terminate is active it means that the terminate, not the 
  // close button, has been pressed
  if ( terminateActive == true )
    {
      long x = (long)m_controller;

      // ask user if they want to stop the job. 1 == No
      int result = m_select->QuitJobMessage();

      // did they say No
      if ( result == 0 )
        {
          return;
        }

      // indicate the current job has been aborted
      m_controller->SetCommandResult( -1 );

      // disable terminate flag
      terminateActive = false;
     }

  // close window
  m_select->close();
}



void SubmitStatusController::updateDisplay( void )
{
#ifdef DEBUG
  cout << this << " SubmitStatusController::updateDisplay()" << endl;
#endif

  // parse string output for memory message
  while ( m_gempisThread->GetOutputAvailable() == true )
    {
      bool allow_change_color = true;
      QString str;
      str = m_gempisThread->GetNextOutputLine();

      QString field1, field2;
      field1 = str.section( ' ', 0, 0, QString::SectionSkipEmpty );
      field2 = str.section( ' ', 1, 1, QString::SectionSkipEmpty );
      // Check for special message format
      if ( field1 == "MeSsAgE" )
        {
          QString field3;
          field3 = str.section( ' ', 2, 2, QString::SectionSkipEmpty );

          if ( field2 == "STATE" )
            {
              m_select->JobInformationStatelineEdit->setText( field3 );
            }
          else if ( field2 == "ERROR" )
            {
              m_errorDetected = true;

              // update Last Error/Warning Messages & Reason
              QString msg;
              msg = str.section( ' ', 1, -1, QString::SectionSkipEmpty );
              m_select->JobInformationLastErrortextEdit->append( msg );
            }
          else if ( field2 == "WARNING" )
            {
              m_warningDetected = true;

              // update Last Error/Warning Messages
              QString msg;
              msg = str.section( ' ', 1, -1, QString::SectionSkipEmpty );
              m_select->JobInformationLastErrortextEdit->append( msg );
            }
          else if ( field2 == "VERBOSE" )
            {
              // do not update the window, 
              // the contents will go to the log file
            }
          else if ( field2 == "JOBID" )
            {
              m_select->JobInformationGempisIdlineEdit->setText( field3 );
            }
          else if ( field2 == "RESULT" )
            {
              // the result is not only determined from this
              if ( field3 != "0" )
                m_result = field3.toInt();
            }
          else
            {
              cout << "Unknown option [" + field2 + "] " + str << endl;
            }
          // these messages do not get put in the job window
          continue;
        }
      else if ( field1 == "Job" )  // an LSF job id in format 'Job <xxx>'
        {
          // remove '<' and '>'
          QString jobid;
          jobid = field2.section( '<', 1, 1 ); 
          jobid = jobid.section( '>', 0, 0 );

          // enable and display if value present
          if ( ! jobid.isEmpty() )
            {
              m_select->JobInformationJobIdlineEdit->setEnabled( true );
              m_select->JobInformationJobIdlineEdit->setText( jobid );
            }
        }
      else // parse this for errors or warnings
        {
          // if a first error has been detected
          if ( m_errorDetected == false && str.contains("ERROR", false) )
            {
#ifdef DEBUG
              cout << this << " SubmitStatusController::updateDisplay() ERROR seen." << endl;
#endif
              // scan list of strings to skip
              bool skip = false;
              for ( QStringList::Iterator it = m_skipErrors.begin(); it != m_skipErrors.end(); ++it ) 
              {
                QString cstr;
                cstr = *it;
                if ( cstr.length() == 0 )
                  {
                    continue;
                  }
                  if ( str.contains(cstr, true)  )
                    {
#ifdef DEBUG
                      cout << this << " SubmitStatusController::updateDisplay() ERROR containing [" << cstr << "] ignored." << endl;
#endif
                      skip = true;
                      allow_change_color = false;
                    }
                }
              if ( skip == false )
                {
#ifdef DEBUG
                  cout << this << " SubmitStatusController::updateDisplay() ERROR detected." << endl;
#endif
                  m_errorDetected = true;
                  // add to last error/warning field
                  m_select->JobInformationLastErrortextEdit->append( str );
                }
            }

          // if an warning has been detected
          if ( m_warningDetected == false && str.contains( "WARNING", false) )
          {
#ifdef DEBUG
            cout << this << " SubmitStatusController::updateDisplay() WARNING seen." << endl;
#endif
            // scan list of strings to skip
            bool skip = false;
            for ( QStringList::Iterator it = m_skipWarnings.begin(); it != m_skipWarnings.end(); ++it ) 
            {
              QString cstr;
              cstr = *it;
              if ( cstr.length() == 0 )
              {
                continue;
              }
              if ( str.contains(cstr, true)  )
              {
#ifdef DEBUG
                cout << this << " SubmitStatusController::updateDisplay() WARNING ignored." << endl;
#endif
                skip = true;
                allow_change_color = false;
              }
            }
            if ( skip == false )
            {
#ifdef DEBUG
              cout << this << " SubmitStatusController::updateDisplay() WARNING detected." << endl;
#endif
              m_warningDetected = true;
              // add to last error/warning field
              m_select->JobInformationLastErrortextEdit->append( str );
            }
          }
        }

      // update last update window with current time
      QDateTime dt;
      dt = m_gempisThread->GetLastUpdate();
      m_select->JobInformationLastResponselineEdit->setText( dt.toString( "hh:mm:ss" ) );

      // display line
      m_select->ApplicationTerminaltextEdit->insertParagraph( str, m_lastLineNumber  );

      // has an error or warning been recognized
      if ( allow_change_color == true )
      {
        // choose background based on text
        if ( str.contains("ERROR", false) )
        {
          QColor r(red);
          // set background to RED
          m_select->ApplicationTerminaltextEdit->setParagraphBackgroundColor( m_lastLineNumber, r );
        }
        else if ( str.contains("WARNING", false) )
        {
          QColor r(yellow);
          // set background to yellow
          m_select->ApplicationTerminaltextEdit->setParagraphBackgroundColor( m_lastLineNumber, r );
        }
      }
      m_lastLineNumber++;
    }
}

