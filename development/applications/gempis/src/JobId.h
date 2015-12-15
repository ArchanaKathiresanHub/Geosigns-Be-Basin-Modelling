///////////////////////////////////////////////////////////////////////////////
// JobId.h - Get the next Gempis job id
//
// Written by: Greg Lackore
//
//

#include <stdio.h>            // printf(), perror(), snprintf(), fopen(), ...
#include <stdlib.h>           // exit()
#include <errno.h>            // errno, EEXIST
#include <unistd.h>           // sleep()

#include <iostream>
#include <fstream>
#include <iomanip>

#include "FileLock.h"

#include <signal.h>             // sigaction

// when active will loop infinitly
//#define DEBUG_GETNEXTJOBID

class JobValue
{
public:
  JobValue();
  ~JobValue();

  // set value
  void Set( const char * );

  // returns current value
  const char * Get( void );

  // prefix
  JobValue& operator++(void);

private:
  unsigned int m_value;
  char m_string[ 16 ];

};


///////////////////////////////////////////////////////////////////////////////
//
// JobId class
//
class JobId
{
public:
  // singleton
  static JobId * Instance(); 

  // set the name of the data and lock file
  bool SetFileNames( const string & datapath = "/tmp/gempis/lastid", 
		     const string & lockpath = "/tmp/gempis/lastid.lock" ); 
  bool SetNext( void );
  const char * Get( void );

protected:
  JobId(); 
  ~JobId();

private:
  static JobId *s_instance;
 
  bool getLock( void );
  bool releaseLock( void );

  bool readData( void );
  bool writeData( void );

  bool m_filesDefined;
  string m_dataFilename;
  JobValue * m_jobValue;
  FileLock * m_fileLock;
  File * m_file;
};


