///////////////////////////////////////////////////////////////////////////////
// File.cpp - See File.h for a description on how to use
//
// Written by: Greg Lackore
//
// This class uses file based lockinis a set of file utilities. 
//
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

#include "File.h"

// activating this will turn on routine messages
//#define DEBUG_FILE


////////////////////////////////////////////////////////////////////////////////
//
// Umask Class public members
//
Umask::Umask()
{
  // sets and save umask
  m_mask = umask( 0 ); 
}

Umask::~Umask()
{
  // sets and save umask
  umask( m_mask );
}


////////////////////////////////////////////////////////////////////////////////
//
// File Class public members
//

// c'tor
File::File( const string & path, bool accessEveryone ) :
  m_umask( 0 ),
  m_filename( "" ),
  m_fd( -1 ),
  m_errno( 0 )
{
#if defined( DEBUG_FILE )
  cout << this << " File::File( " << path << " )" << endl;
#endif

  m_filename = path;

  // the file buffer is used by system commands.
  m_filebuffer = (char *)malloc( 1024 );
  path.copy( m_filebuffer, path.length() );

  if ( accessEveryone == true )
  {
    // unlock file mask
    m_umask = new Umask;
  }
}


// d'tor
File::~File( void )
{
#if defined( DEBUG_FILE )
  cout << this << " File::~File( " <<  m_filename << " )" << endl;
#endif

  delete m_filebuffer;

  if ( m_umask != 0 )
  {
    // restore umask
    delete m_umask;
  }
}


// check if a file exist
bool File::Exists( void )
{
#if defined( DEBUG_FILE )
  cout << this << " File::Exists( " <<  m_filename << " )" << endl;
#endif

  // stat file, if not there is an error
  struct stat data;
  int result = stat( m_filebuffer, &data ); 
  if ( result == -1 )
  {
    return false;
  }

  // reset error flag
  m_errno = 0;

  return true;
}


// create the file
bool File::Create( void )
{
#if defined( DEBUG_FILE )
  cout << this << " File::Create( " << m_filename << " )" << endl;
#endif

  // create it
  m_fd = creat( m_filebuffer, O_RDWR
      	       | S_IRUSR | S_IWUSR   // R/W permission owner
    	       | S_IRGRP | S_IWGRP   // R/W permission group
    	       | S_IROTH | S_IWOTH );// R/W permission world

  if ( m_fd == -1 )
  {
    m_errno = errno;
    return false;
  }

  // reset error flag
  m_errno = 0;

  return true;
}


// opens file
bool File::Open( void )
{
#if defined( DEBUG_FILE )
  cout << this << " File::Open( " << m_filename << " )" << endl;
#endif

  m_fd = open( m_filebuffer, O_RDWR );
  if ( m_fd == -1 )
  {
    m_errno = errno;
    return false;
  }

  // reset error flag
  m_errno = 0;

  return true;
}


// close file
bool File::Close( void )
{
#if defined( DEBUG_FILE )
  cout << this << " File::Close( " << m_filename << " )" << endl;
#endif

  // generate the fcntl id based on the key value
  int result = close( m_fd ); 
  if ( result == -1 )
  {
    m_errno = errno;
    return false;
  }

  m_fd = -1;

  // reset error flag
  m_errno = 0;

  return true;
}


// set file pointer to offset in file
bool File::Seek( int offset )
{
#if defined( DEBUG_FILE )
  cout << this << " File::Seek( " << m_filename << ", " << offset << " )" << endl;
#endif

  if ( m_fd == -1 )
  {
    return false;
  }

  // set file pointer to offset
 if ( fcntl(m_fd, F_SETFD, offset) == -1 )
  {
    m_errno = errno;
    return false;
  }

  // reset error flag
  m_errno = 0;

  // return success
  return true;
}

// attempts to write lock (fnctl) a file
bool File::WriteLock( void )
{
#if defined( DEBUG_FILE )
  cout << this << " File::WriteLock( " << m_filename << " )" << endl;
#endif

  if ( m_fd == -1 )
  {
    return false;
  }

  // flock control data
  struct flock data;
  data.l_type   = F_WRLCK;
  data.l_whence = SEEK_SET;
  data.l_start  = 0;
  data.l_len    = 0;
  data.l_pid    = 0;
 
  // set write lock, wait until its achieved
 if ( fcntl(m_fd, F_SETLKW, &data) == -1 )
  {
    m_errno = errno;
    return false;
  }

  // reset error flag
  m_errno = 0;

  // return success
  return true;
}


// read unsigned integer from file
bool File::Read( unsigned int * value )
{
#if defined( DEBUG_FILE )
  cout << this << " File::ReadUInt( " << m_filename << " )" << endl;
#endif

  // generate the fcntl id based on the key value
  unsigned int data;
  int result = read( m_fd, &data, sizeof(int) ); 
  if ( result == -1 )
  {
    m_errno = errno;
    return false;
  }

  *value = data;
  return true;
}


// read unsigned integer from file
bool File::Read( int size, char * buffer )
{
#if defined( DEBUG_FILE )
  cout << this << " File::ReadStr( " << m_filename << " )" << endl;
#endif

  // generate the fcntl id based on the key value
  int result = read( m_fd, buffer, size ); 
  if ( result == -1 )
  {
    m_errno = errno;
    return false;
  }
#if defined( DEBUG_FILE )
  cout << " = " << buffer << endl;
#endif
  return true;
}


// read jobid from file
// assume that lock was set
bool File::Write( unsigned int value )
{
#if defined( DEBUG_FILE )
  cout << this << " File::WriteUInt( " << m_filename << " )" << endl;
#endif

  // generate the fcntl id based on the key value
  int result = write( m_fd, &value, sizeof(int) ); 
  if ( result == -1 )
  {
    m_errno = errno;
    return false;
  }
  return true;
}



// read jobid from file
// assume that lock was set
bool File::Write( const char * buffer )
{
#if defined( DEBUG_FILE )
  cout << this << " File::WriteStr( " << m_filename << ", " << buffer << ")" << endl;
#endif

  // generate the fcntl id based on the key value
  int result = write( m_fd, buffer, strlen(buffer)+1 ); 
  if ( result == -1 )
  {
    m_errno = errno;
    //cout << strerror(errno) << endl;
    return false;
  }
  return true;
}

// returns last errno value
string & File::Name( void )
{
  // copy filename string into something unix can use
  //char buffer[ 1024 ];
  //m_filename.copy( buffer, m_filename.length() );

  return m_filename;
}


// returns last errno value
void File::PrintError( void )
{
  cerr << "File::PrintError " << m_filename << ": " << strerror(m_errno) << "." << endl;
}

