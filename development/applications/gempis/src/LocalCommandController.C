///
///
///
/// To do: when resubmitting the application it crashes because m_commandString is 
/// corrupted somewhere. So to avoid this we 
/// reload the job file, reset m_commandString

#include "includestreams.h"

// project includes
#include <JobLauncherController.h>
#include <LocalCommandController.h>
#include <LocalCommandUI.hpp>
#include <GempisThread.h>

// Qt includes
#include <qlineedit.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qtextedit.h> 
#include <qpushbutton.h> 
#include <qapplication.h> 
#include <qdatetime.h> 
#include <qtimer.h> 
#include <qmessagebox.h> 

// process info
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>


#define LC_TIMEOUT 1000  // timeout value for refresh



LocalCommandController::LocalCommandController( LocalCommandUI * select ) :
  cancelActive( 1 ),
  m_result( 0 ),
  m_select( select ), 
  m_controller( 0 )
{
#ifdef DEBUG
  cout << this << " LocalCommandController::LocalCommandController()" << endl;
#endif

  // create gempis data and job thread
  m_gempisThread = new GempisThread();

  // define activation signal for Cancel button
  connect( m_select->CancelpushButton, SIGNAL( clicked() ), this, SLOT( cancelClicked() ) ) ;

  // start a timer to check the cancel button
  internalLCTimer = new QTimer( this );

  m_select->setModal( false );
  m_select->show();
}

LocalCommandController::~LocalCommandController()
{
#ifdef DEBUG
  cout << this << " LocalCommandController::~LocalCommandController()" << endl;
#endif

  // stop , disconnect and delete update timer
  internalLCTimer->stop();
  disconnect( internalLCTimer, SIGNAL(timeout()), this, SLOT(refreshWindow()) );
  delete internalLCTimer;

  // wait for thread to complete, is probably already done
  m_gempisThread->wait();
  delete m_gempisThread;
}


void LocalCommandController::StartLocalCommand( JobLauncherController * ptr ) 
{
  QString command;

#ifdef DEBUG
  cout << this << " LocalCommandController::StartLocalCommand()" << endl;
#endif

  // define control information pointer
  m_controller = ptr;

  // get command and route stderr to stdout
  command = m_controller->GetCommand();

  // update display with command
  m_select->TerminaltextEdit->insertParagraph( command, -1 );

  // define the Gempis command and logfile
  m_gempisThread->SetCommand( command );
  m_gempisThread->SetLogFileName( m_controller->GetProjectLogFileName() );

  // start update timer
  internalLCTimer->singleShot( LC_TIMEOUT, this, SLOT(refreshWindow()) );

  // spawn child to execute command
  m_gempisThread->start();
}


void LocalCommandController::KillChildExecutable( void )
{
#ifdef DEBUG
  cout << this << " LocalCommandController::KillChildExecutable()" << endl;
#endif

  // tell the Gempis thread to kill itself
  m_gempisThread->KillThread();
}


void LocalCommandController::refreshWindow( void )
{
#ifdef DEBUG
  cout << this << " LocalCommandController::refreshWindow()" << endl;
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
    }
  else
    {
      // restart timer 
      internalLCTimer->singleShot( LC_TIMEOUT, this, SLOT(refreshWindow()) );
    }
}


void LocalCommandController::commandComplete( void )
{
  QString stg;

#ifdef DEBUG
  cout << this << " LocalCommandController::commandComplete()"<< endl;
#endif

  if ( cancelActive )
    {
      m_controller->SetCommandResult( m_result );
    }

  m_select->CancelpushButton->setText( "Close" );

  // if successful, we don't want to see anything, exit this window
  if ( m_result )
  {
    QString str;
    QString num;

    cancelActive = 0;

    num.setNum( m_result );
    str = "Error ";
    str += num;

    m_select->ExecutionResultlineEdit->setText( str );
  }
  else
  {
    m_select->close();
    // make sure that nothing else happens after this function
    return;
  }
}


void LocalCommandController::cancelClicked( void )
{
#ifdef DEBUG
  cout << this << " LocalCommandController::cancelClicked()" << endl;
#endif

  // if cancel is active it means that the Cancel, not the 
  // close button, has been pressed
  if ( cancelActive )
    {
      // indicate the current job has been aborted
      m_controller->SetCommandResult( -1 );

      // diasble cancel flag
      cancelActive = 0;
    }

  // close window
  m_select->close();
  // make sure that nothing else happens after this function
}


void LocalCommandController::updateDisplay( void )
{
#ifdef DEBUG
  cout << this << " LocalCommandController::updateDisplay()" << endl;
#endif

  // get data from thread
  while ( m_gempisThread->GetOutputAvailable() == true )
    {
      QString str;
      str = m_gempisThread->GetNextOutputLine();

      // update window
      m_select->TerminaltextEdit->insertParagraph( str, -1 );
    }
}


