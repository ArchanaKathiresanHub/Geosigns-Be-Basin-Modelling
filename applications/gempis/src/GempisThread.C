///
///
///
/// Child process are terminated using the Hang Up (1) suignal
///

#include "includestreams.h"

#include <SubmitStatusController.h>
#include <GlobalData.h>
#include <LogFile.h>
#include <GempisThread.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qtextedit.h> 
#include <qpushbutton.h> 
#include <qapplication.h> 
#include <qdatetime.h> 
#include <qtimer.h> 
#include <qfile.h> 
#include <qmessagebox.h> 

// process info
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>             // getpwuid
#include <signal.h>          // sigaction


///////////////////////////////////////////////////////////////////////
//
//
//
GempisThread::GempisThread( void ) :
  m_running( false ),
  m_result( 0 ),
  m_logFileName( "" ),
  m_command( "" ),
  m_outputLastWrite( 0 ),
  m_outputLastRead( 0 )
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::GempisThread()" << endl;
#endif

  // set last update with current time
  m_lastUpdate = QDateTime::currentDateTime();
}


GempisThread::~GempisThread()
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::~GempisThread()" << endl;
#endif
}


///////////////////////////////////////////////////////////////////////
//
//
void GempisThread::SetCommand( const QString &command )
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::setCommand( " << command << " )" << endl;
#endif

  m_command = command;
}


void GempisThread::SetLogFileName( const QString &fileName )
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::setLogFileName( " << fileName.latin1() << " )" << endl;
#endif

  m_logFileName = fileName;
}


///////////////////////////////////////////////////////////////////////
//
//
bool GempisThread::GetOutputAvailable()
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::getNextOutputAvailable()" << endl;
#endif

  // no new data?
  if ( m_outputLastRead >= m_outputLastWrite )
    {
      return false;
    }

  // say we have data
  return true;
}


QString GempisThread::GetNextOutputLine()
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::getNextOutputLine()" << endl;
#endif

  // no new data?
  if ( m_outputLastRead >= m_outputLastWrite )
    {
      return "";
    }

  // copy latest error lines over
  QString nextline;

  m_mutex.lock();

  nextline = m_outputText[ m_outputLastRead++ ];

  m_mutex.unlock();

  // say we have data
  return nextline;
}


///////////////////////////////////////////////////////////////////////
//
//
bool GempisThread::GetRunning( void )
{
#ifdef DEBUG_GEMPISTHREAD  // a failure is not critical, only inconvient
  cout << this << " GempisThread::getRunning()" << endl;
#endif

  return m_running;
}


long GempisThread::GetResult( void )
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::getResult()" << endl;

#endif

  return m_result;
}


QDateTime GempisThread::GetLastUpdate( void )
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::GetLastUpdate()" << endl;
#endif

  return m_lastUpdate;
}


///////////////////////////////////////////////////////////////////////
//
//
//
void GempisThread::KillThread( void )
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::KillThread()" << endl;
#endif

  LogFile * lf = new LogFile( m_logFileName );
  lf->WriteLine( "************************************************************" );
  lf->WriteLine( "-TeRmInAtE Thread Request" );
  char hostnm[ 128 ];
  gethostname( hostnm, 124 );
  lf->Write( "-TeRmInAtE on Hostname:     " );
  lf->WriteLine(  hostnm );

  // get the command path
  QString command;
  command = GlobalData::GetPath();
  lf->Write( "-TeRmInAtE Global Path:     " );
  lf->WriteLine( command );
  if ( command.length() == 0 )
    {
      // if not defined use the EXEDIR environment variable
      cout << "Global data did not work, using EXEDIR variable for pstree path." << endl;
      command = getenv( "EXEDIR" );
      lf->Write( "-TeRmInAtE Using EXEDIR:    " );
      lf->WriteLine( command );
    }

  // add the pstree command
  command += "/pstree -w ";

  // get my process id and add to command
  long pid;
  pid = (long)getpid();
  QString str;
  str.setNum( pid );
  command += str;

  // execute pstree command
#ifdef DEBUG_GEMPISTHREAD
  cout << "Calling command [" << command << "]" << endl;
#endif
  FILE *fp;
  fp = popen( command.latin1(), "r" );
  if  ( fp == NULL )
    {
      cout << "popen [" << command << "] failed." << endl;
      lf->Write( "-TeRmInAtE Command failed:  " );
      lf->WriteLine( command );
      delete lf;
      return;
    }

  // skip first process, that's me
  char * buffer = new char [ BufferSize ];
  fgets( buffer, BufferSize-1, fp );
#ifdef DEBUG_GEMPISTHREAD
  cout << "First line " << buffer;
#endif
  QString userid;
  str = buffer;
  lf->Write( "-TeRmInAtE My process:      " );
  lf->WriteRaw( str );     // contains an <LF>
  userid = str.section( ' ', 1, 1, QString::SectionSkipEmpty );
  if ( userid == NULL )
    {
      // if no value then something has gone wrong
      cout << "Error! Cannot determine my process id. Processes cannot be killed." << endl;
      lf->WriteLine( "-TeRmInAtE ERROR:           Users process id could not be determined." );
      delete lf;
      return;
    }

 // parse the rest of ther output
  while ( fgets(buffer, BufferSize-1, fp) != NULL )
  {
    // remove carriage return
    buffer[ strlen(buffer)-1 ] = '\0';
    str = buffer;
#ifdef DEBUG_GEMPISTHREAD
    cout << endl << "Read [" << str << "]" << endl;
#endif

    lf->Write( "-TeRmInAtE Child detected:  " );
    lf->WriteLine( str );

    // get process id, don't try to kill an undefined PID
    QString cpid;
    cpid = str.section( ' ', 0, 0, QString::SectionSkipEmpty );
    if ( cpid == NULL )
      {
        lf->WriteLine( "-TeRmInAtE Skipping undefined process string."  );
        continue;
      }
    // don't kill pid 0, that would kill everything in the process group
    pid_t pi;
    pi = (pid_t)(cpid.toLong());
    if ( ! pi )
      {
        lf->WriteLine( "-TeRmInAtE Skipping process id 0."  );
        continue;
      }

    // get userid, skip if undefined or I am not owner
    QString usr;
    usr = str.section( ' ', 1, 1, QString::SectionSkipEmpty );
    if ( usr == NULL || usr != userid )
      {
        lf->WriteLine( "-TeRmInAtE Skipping undefined user name."  );
        continue;
      }

    // get executable name
    QString exec;
    exec = str.section( ' ', 2, 2, QString::SectionSkipEmpty );
#ifdef DEBUG_GEMPISTHREAD
    cout << "The executable is [" << exec << "]" << endl;
#endif
    if ( exec == NULL )
      {
        lf->WriteLine( "-TeRmInAtE Skipping undefined executable."  );
        continue;
      }
    // don't kill myself, children are owned by root process
    if ( exec.contains("pstree") )
      {
        lf->WriteLine( "-TeRmInAtE Skipping pstree."  );
        continue;
      }

    // send Hang Up to terminate child process
    lf->Write( "-TeRmInAtE *** Hangup(" );
    lf->Write( pi );
    lf->Write( "):" );
    lf->WriteLine( exec );
    kill( pi, SIGHUP );
  }

  QDateTime dt = QDateTime::currentDateTime();
  lf->Write( "-TeRmInAtE Completed on:    " );
  lf->WriteLine( dt.toString( "ddd MMMM d, yyyy hh:mm:ss" ) );
  delete lf;

  // remove terminal buffer
  delete [] buffer;

  fflush( fp );
  pclose( fp );
}


///////////////////////////////////////////////////////////////////////
//
// Qt pure virtual function
//
// start the thread in the foreground
void GempisThread::run( void )
{
#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::run()" << endl;
#endif

  // append a redirect errout to stdout onto the command
  m_command += " 2>&1";

#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::run() command is [" << m_command << "]" << endl;
#endif

  // execute command 
  FILE * fp;
  fp = popen( m_command.latin1(), "r" );
  if ( fp == NULL )
    {
      cout << "Command " << m_command << " failed on popen." << endl;

      // signal the SubmitStatus that the command is complete
      m_result = 1;

      return;
    }

  // flag that we are not running
  m_running = true;

  // add a header to log file
  LogFile * lf = new LogFile( m_logFileName );
  //  lf->Append();
  lf->WriteLine( "==============================================================" );
  lf->WriteLine( "-ThReAd Entry" );
  struct passwd *ppw;
  ppw = getpwuid( getuid() );
  lf->Write( "-ThReAd User:         " );
  lf->WriteLine( ppw->pw_name );
  char hostnm[ 128 ];
  gethostname( hostnm, 124 );
  lf->Write( "-ThReAd Hostname:     " );
  lf->WriteLine(  hostnm );
  lf->Write( "-ThReAd Command:      " );
  lf->WriteLine( m_command );
  QDateTime dt;
  dt = QDateTime::currentDateTime();
  lf->Write( "-ThReAd Started on:   " );
  lf->WriteLine( dt.toString( "ddd MMMM d, yyyy hh:mm:ss" ) );
  lf->WriteLine( "---------- Start Thread --------------------------------------" );

  // create an character buffer for terminal output
  char * buffer = new char [ BufferSize ];

  // get line, is it finished
  while ( true )
    {
      if( fgets(buffer, BufferSize-1, fp) == NULL )
        {
          break;
        }

      // remove <LF>
      buffer[ strlen(buffer)-1 ] = '\0';

      lf->WriteLine( buffer );

      // lock mutex
      m_mutex.lock();

      // add data to output buffer
      m_outputText.push_back( buffer );
      m_outputLastWrite++;

      // unlock mutex
      m_mutex.unlock();

      // set last update with current time
      m_lastUpdate = QDateTime::currentDateTime();
    }

  // remove terminal buffer
  delete [] buffer;

  fflush( fp );

  // get the return status of the command. If there was no 
  // explicit assignment then assign the result of the command
  int res = pclose( fp );
  if ( res == -1 )
    {
      lf->Write( "-ThReAd ERROR:  pclose failed error = " );
      lf->WriteLine( res );
      if ( ! m_result )
        {
          m_result = res;
        }
    }

  // give job summary 
  lf->WriteLine( "---------- End Thread ----------------------------------------" );
  dt = QDateTime::currentDateTime();
  lf->Write( "-ThReAd Completed on: " );
  lf->WriteLine( dt.toString( "ddd MMMM d, yyyy hh:mm:ss" ) );
  lf->Write( "-ThReAd Result:       " );
  lf->WriteLine( m_result );
  delete lf;

  m_running = false;

#ifdef DEBUG_GEMPISTHREAD
  cout << this << " GempisThread::run() result was " << m_result << "." << endl;
#endif
}


