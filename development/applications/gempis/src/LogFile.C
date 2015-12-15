///
///
///

#include "includestreams.h"

#include <LogFile.h>

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


///////////////////////////////////////////////////////////////////////
//
//
LogFile::LogFile( const QString &fileName ) :
  m_f( 0 )
{
#ifdef DEBUG_LOGFILE
  cout << this << " LogFile::LogFile(" << fileName << ")" << endl;
#endif

  m_logFileName = fileName;

  if ( ! m_logFileName.isEmpty() )
    {
      m_f = new QFile( m_logFileName );
    }
}


///////////////////////////////////////////////////////////////////////
//
//
LogFile::~LogFile()
{
#ifdef DEBUG_LOGFILE
  cout << this << " LogFile::~LogFile()" << endl;
#endif

  // close log file
  if ( m_f )
    {
      delete m_f;
      m_f = 0;
    }
}


///////////////////////////////////////////////////////////////////////
//
//
void LogFile::Reset( void )
{
#ifdef DEBUG_LOGFILE
  cout << this << "Reset()" << endl;
#endif

  if ( ! m_logFileName.isEmpty() )
    {
      if ( ! m_f->open( IO_WriteOnly | IO_Truncate ) )
        {
          cout << "Warning! Could not reset log file " << m_logFileName << endl;
        }
      m_f->close();
    }
}


///////////////////////////////////////////////////////////////////////
//
//
void LogFile::Append( void )
{
#ifdef DEBUG_LOGFILE
  cout << this << " Append()" << endl;
#endif

  // open the log file, set pointer to end
  /*
  if ( ! m_logFileName.isEmpty() )
    {
      if ( m_f->open( IO_WriteOnly | IO_Append ) )
        {
          cout << "Warning! Could not append to log file " << m_logFileName << endl;
        }
    }
  */
}


///////////////////////////////////////////////////////////////////////
//
//
void LogFile::WriteRaw( const char buffer[] )
{
#ifdef DEBUG_LOGFILE
  cout << this << " LogFile::WriteRaw()" << endl;
#endif

  if ( ! m_logFileName.isEmpty() )
    {
      // check if log file is open
      if ( m_f->open( IO_WriteOnly | IO_Append ) )
        {
          QTextStream ts( m_f );
          // copy line (w/ <LF>) to log file
          ts.writeRawBytes( buffer, strlen(buffer) );
          m_f->close();
        }
    }
}


///////////////////////////////////////////////////////////////////////
//
//
void LogFile::Write( const QString & str )
{
#ifdef DEBUG_LOGFILE
  cout << this << " LogFile::Write()" << endl;
#endif

  if ( ! m_logFileName.isEmpty() )
    {
      // check if log file is open
      if ( m_f->open( IO_WriteOnly | IO_Append ) )
        {
          QTextStream ts( m_f );
          ts << str;
          m_f->close();
        }
    }
}


void LogFile::Write( int val )
{
#ifdef DEBUG_LOGFILE
  cout << this << " LogFile::Write()" << endl;
#endif

  if ( ! m_logFileName.isEmpty() )
    {
      // check if log file is open
      if ( m_f->open( IO_WriteOnly | IO_Append ) )
        {
          QTextStream ts( m_f );
          ts << val;
          m_f->close();
        }
    }
}

///////////////////////////////////////////////////////////////////////
//
//
void LogFile::WriteLine( const QString & str )
{
#ifdef DEBUG_LOGFILE
  cout << this << " LogFile::WriteLine()" << endl;
#endif

  if ( ! m_logFileName.isEmpty() )
    {
      // check if log file is open
      if ( m_f->open( IO_WriteOnly | IO_Append ) )
        {
          QTextStream ts( m_f );
          ts << str << endl;
          m_f->close();
        }
    }
}

void LogFile::WriteLine( int val )
{
#ifdef DEBUG_LOGFILE
  cout << this << " LogFile::WriteLine()" << endl;
#endif

  if ( ! m_logFileName.isEmpty() )
    {
      // check if log file is open
      if ( m_f->open( IO_WriteOnly | IO_Append ) )
        {
          QTextStream ts( m_f );
          ts << val << endl;
          m_f->close();
        }
    }
}



