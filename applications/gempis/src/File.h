#ifndef _FILE_H_
#define _FILE_H_
///////////////////////////////////////////////////////////////////////////////
// File - a class that performs operation on files 
//
// Written by: Greg Lackore
//
//

#include <sys/types.h>        // mode_t
#include <sys/stat.h>

using namespace std;


// Umask class - set and restore umask. Note the file umask will be
// set to 0 unstead of 022 as long as Umask is defined.
//
class Umask
{
public:
  Umask();
  ~Umask();

private:
  mode_t m_mask;
};


// File class
//
class File
{

public:
  File( const string & path, bool access_everyone ); 
  ~File();

  // file based member functions
  bool Exists( void );
  bool Create( void );
  bool Open( void );
  bool Close( void );
  bool Seek( int );
  bool WriteLock( void );
  bool Read( unsigned int * );
  bool Read( int size, char * );
  bool Write( unsigned int );
  bool Write( const char * );
  string & Name( void );
  void PrintError( void );

private:
  Umask * m_umask;
  string m_filename;
  char * m_filebuffer;
  int m_fd;
  int m_errno;
};


#endif // _FILE_H_
