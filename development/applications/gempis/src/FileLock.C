///////////////////////////////////////////////////////////////////////////////
// FileLock.C - See FileLock.h for a description on how to use
//
// Written by: Greg Lackore
//
// This class uses file based locking. 
//
// Note: It is assumed that the locking will only be done once per process.
//       Otherwise a 1 second sleep must be done before trying again.
//

#include <stdio.h>            // printf(), perror(), snprintf(), fopen(), ...
#include <stdlib.h>           // exit(), rand()
#include <errno.h>            // errno, EEXIST
#include <string.h>           // strcpy()
#include <fcntl.h>            // fcntl()
#include <sys/types.h>        // stat()
#include <sys/stat.h>
#include <unistd.h>


#include <iostream>
#include <fstream>
#include <iomanip>

#include "FileLock.h"

// activating this will turn on routine messages
//#define DEBUG_FILELOCK

// wait a random time < ( max + 1 ) seconds
#define USECS 1000000          // usecs in 1 second
// < 1000000 on some systems is an error
void randomWait( int max )
{
  // calculate duration to wait, normalized to max random value
  unsigned long dur = rand();
  dur *= USECS;
  dur /= RAND_MAX;
  dur *= max;
  if ( dur < USECS )
  {
    dur += USECS;   
  }

#if defined( DEBUG_FILELOCK )
  cout << "Sleeping " << dur << " usecs." << endl;
#endif

  usleep( dur );
}


////////////////////////////////////////////////////////////////////////////////
//
// FileLock Class public members
//


// initialize   
//
// 
// Returns: 0 - sucess
//         otherwise - error
//
FileLock::FileLock( const string & path ) :
  m_havelock( false ),
  m_status( true )
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::FileLock(" << path << ")" << endl;
#endif

  // create file lock accessible to everyone
  m_file = new File( path, true );

  // create lock file
  if ( createLock() == false )
  {
    /* 
       do not write errors. this will be read as a jobid
    cerr << this << " FileLock::FileLock - unable to create lock file ";
    m_file->PrintError();
    */
  }
}

// d'tor
FileLock::~FileLock()
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::~FileLock( " << m_file->Name() << " )" << endl;
#endif

  Release();

  delete m_file;
}


//
// calling process will pend if write lock fails
//
// Returns: true  - control returned and lock aquired,
//          false - control returned but acquire failed, caused most
//                  likely by signal being sent to process. The calling
//                  process should terminate as quickly as possible.
bool FileLock::Acquire( void )
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::Acquire( " << m_file->Name() << " )" << endl;
#endif
  
  if ( openLock() == false )
  {
    /*
      no output on error
    cerr << "FileLock::Aquire error. Could not open lock file ";
    m_file->PrintError();
    */
    return false;
  }

  bool result = setLock();
  if ( result == false )
  {
    /*
    cerr << "FileLock::Aquire error. ";
    m_file->PrintError();
    */
  }
  else
  {
    m_havelock = true;
  }

  return result;
}


//
// this routine is non-blocking
bool FileLock::Release( void )
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::Release( " << m_file->Name() << " )" << endl;
#endif

  if ( m_havelock == false )
  {
    return true;
  }

  bool result = closeLock();
  /*
  if ( result == false )
  {
    cerr << "FileLock::Release error. ";
    m_file->PrintError();
  }
  */

  m_havelock = false;

  return result;
}


// returns whether the status so far was successful
//
//
bool FileLock::GetStatus( void )
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::GetStatus( " << m_file->Name() << " )" << endl;
#endif

  return m_status;
}


// Class private members
//

// create the lock file
bool FileLock::createLock( void )
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::createLock( " << m_file->Name() << " )" << endl;
#endif

  m_status = true;

  // only create file if it does not already exist
  if ( m_file->Exists() == true )
  {
    return true;
  }

  // backoff a random time and try again
  randomWait( 3 );

  // try again
  if ( m_file->Exists() == true )
  {
    return true;
  }

  // create it
  if ( m_file->Create() == false )
  {
    // the create may fail, but succeed later on
    // m_status = false;
    return false;
  }

  m_file->Close();

  return true;
}


// create the lock file
bool FileLock::openLock( void )
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::openLock( " << m_file->Name() << " )" << endl;
#endif

  m_status = true;

  if ( m_file->Open() == false )
  {
    m_status = false;
  }
  return m_status;
}


// close file (releases lock)
bool FileLock::closeLock( void )
{ 
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::closeLock( " << m_file->Name() << " )" << endl;
#endif

  // close lock file
  m_status = m_file->Close(); 

  return m_status;
}


// attempts to create a fcntl
bool FileLock::setLock( void )
{
#if defined( DEBUG_FILELOCK )
  cout << this << " FileLock::setLock( " << m_file->Name() << " )" << endl;
#endif

  if ( m_status == false )
  {
    return false;
  }

  // set write lock, wait until its achieved
  m_status = m_file->WriteLock();

  return m_status;
}


