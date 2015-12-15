///////////////////////////////////////////////////////////////////////////////
// GetNextJobId.C - Get the next Gempis job id
//
// Written by: Greg Lackore
//
//

#include <stdio.h>            // printf(), perror(), snprintf(), fopen(), ...
#include <stdlib.h>           // exit()
#include <string.h>           // strlen()
#include <errno.h>            // errno, EEXIST

#include <iostream>
#include <fstream>
#include <iomanip>

#include "FileLock.h"
#include "JobId.h"

// when active will loop infinitly
//#define DEBUG_JOBID


///////////////////////////////////////////////////////////////////////////////
//
// JobValue class
//

// c'tor
JobValue::JobValue()
{
  Set( "" );
}

// d'tor
JobValue::~JobValue()
{
}

// prefix increment
JobValue & JobValue::operator++( void )
{
  if ( ++m_value > 99999999 )
  {
    m_value = 0;
  }

  // set string value
  sprintf( m_string, "%d", m_value );

  return *this;
}

// returns string value
void JobValue::Set( const char * str )
{
  if ( strlen(str) == 0 )
  {
    // if no value exists, set to first value
    // consider setting to a random number
    m_value = 0;

    // also sets string value
    ++m_value;
  }
  else
  {
    m_value = atoi( str );
  }

  // set string value
  sprintf( m_string, "%d", m_value );
}

// returns string value
const char * JobValue::Get( void )
{
  return m_string;
}


///////////////////////////////////////////////////////////////////////////////
///tmp/gempis/lastid
// JobId class
//

// singleton instance 
JobId* JobId::s_instance = 0;

// c'tor
JobId::JobId() :
  m_filesDefined( false ),
  m_dataFilename( "" ),
  m_fileLock( 0 ),
  m_file( 0 ) 
{
#ifdef DEBUG_JOBID
  cout << "JobId:JobId()" << endl;
#endif

  m_jobValue = new JobValue();
}


// d'tor
JobId::~JobId( void )
{
#ifdef DEBUG_JOBID
  cout << "JobId:~JobId()" << endl;
#endif

  /*
  if ( m_dataFilename != 0 )
  {
    free( m_dataFilename );
  }
  */
  if ( m_fileLock != 0 )
  {
    delete m_fileLock;
  }
  delete m_jobValue;
}


// defines singleton instance
JobId * JobId::Instance()
{
#ifdef DEBUG_JOBID
  cout << "JobId:Instance()" << endl;
#endif

  if ( s_instance == 0 )
  { 
    s_instance = new JobId;
  }
  return s_instance;
}


// set the name of the lock and data file, freezes values once set
bool JobId::SetFileNames( const string & datapath, const string & lockpath )
{
#ifdef DEBUG_JOBID
  cout << "JobId:SetFileNames(" << datapath << ", " << lockpath << ")" << endl;
#endif

  // can only define the file once
  if ( m_filesDefined == true )
  {
    return false;
  }

  // create file lock
  m_fileLock = new FileLock( lockpath );

  // define location of data file
  m_dataFilename = datapath;

  // indicate that files have been defined
  m_filesDefined = true;

  return true;
}


// increments the jobid and saves, return value if successful
bool JobId::SetNext( void )
{
#ifdef DEBUG_JOBID
  cout << "JobId:SetNext()" << endl;
#endif

  if ( m_filesDefined == false )
  {
    return false;
  }
  if ( getLock() == false || readData() == false )
  {
    return false;
  }

  ++(* m_jobValue);

  bool result = writeData();
  if ( releaseLock() == false || result == false )
  {
    return false;
  }
  return true;
}


// get current job value
const char * JobId::Get( void )
{
#ifdef DEBUG_JOBID
  cout << "JobId::Get()" << endl;
#endif

  return m_jobValue->Get();
}


///////////////////////////////////////////////////////////////////////////////
//
// private members
//

// gets lock to jobid 
bool JobId::getLock( void )
{
#ifdef DEBUG_JOBID
  cout << "JobId:getLock()" << endl;
#endif

  if ( m_fileLock->Acquire() == false )
  {
#ifdef DEBUG_JOBID
    cout << "    Acquire failed" << endl;
#endif
    return false;
  }

  // create data file if it doesn't exist so everyone can read it
  m_file = new File( m_dataFilename, true );
  if ( m_file->Exists() == false )
  {
    // creates empty file
    if ( m_file->Create() == false )
    {
#ifdef DEBUG_JOBID
    cout << "    Create failed" << endl;
#endif
      return false;
    }
    // write job id value
    //cout << m_jobValue->Get() << endl;
    if ( m_file->Write( m_jobValue->Get() ) == false )
    {
#ifdef DEBUG_JOBID
    cout << "    Write failed" << endl;
#endif
      return false;
    }
    m_file->Close();
  }
  return true;
}


// release lock to jobid
bool JobId::releaseLock( void )
{
#ifdef DEBUG_JOBID
  cout << "JobId:releaseLock()" << endl;
#endif

  // remove access to data file
  delete m_file;
  m_file = 0;

  if ( m_fileLock->Release() == false )
  {
#ifdef DEBUG_JOBID
    cout << "    Release failed." << endl;
#endif
    return false;
  }

  return true;
}


// read jobid data file
bool JobId::readData( void )
{
#ifdef DEBUG_JOBID
  cout << "JobId:ReadData()" << endl;
#endif

  if ( m_file == 0 || m_file->Open() == false )
  {
#ifdef DEBUG_JOBID
    cout << "    file def or open failed" << endl;
#endif
    return false;
  }

  char buffer[ 16 ];
  if ( m_file->Read( sizeof(buffer), buffer ) == false )
  {
#ifdef DEBUG_JOBID
    cout << "    read failed." << endl;
#endif
    return false;
  }
  m_file->Close();

  m_jobValue->Set( buffer );

  return true;
}


// write jobid data file
bool JobId::writeData( void )
{
#ifdef DEBUG_JOBID
  cout << "JobId:writeData()" << endl;
#endif

  if ( m_file == 0 || m_file->Open() == false )
  {
#ifdef DEBUG_JOBID
    cout << "    file def or open failed." << endl;
#endif
    return false;
  }

  // set pointer to beginning of file
  m_file->Seek( 0 );

  // write job value, terminate end of line with <LF>
  char buffer[ 16 ];
  sprintf( buffer, "%s\n", m_jobValue->Get() );
  if ( m_file->Write( buffer ) == false )
  {
#ifdef DEBUG_JOBID
    cout << "    write failed" << endl;
#endif
    return false;
  }

  m_file->Close();
  return true;
}

