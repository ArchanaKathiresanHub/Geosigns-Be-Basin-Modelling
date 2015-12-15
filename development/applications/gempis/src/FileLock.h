#ifndef _FILELOCK_H_
#define _FILELOCK_H_
///////////////////////////////////////////////////////////////////////////////
// FileLock - a class that creates a file lock using fcntl accessible by other 
//             processes on the local computer.  
//
// Written by: Greg Lackore
//
// Acquire     - member function that does the resource request which blocks 
//               when the resources are not available. 
// Release     - will return the resource. 
// GetStatus   - indicates whether the action was successful so far
//


#include <ostream>            // ostream 

#include <sys/types.h>        // mode_t
#include <sys/stat.h>

#include "File.h"

using namespace std;

// FileLock class
//
class FileLock
{

public:
  // Set the initial number of resources. This will only set the counter when 
  // the process calling it has also created the fcntl.
  FileLock( const string & path = "/tmp/CoreBinder" ); 
  ~FileLock();

  // Requests resources from the system, only will return when the resources
  // have been granted
  bool Acquire( void );

  // Release resources
  bool Release( void );

  // return the last errno value encountered. Will be reset after each operation
  bool GetStatus( void );

private:
  File * m_file;
  bool m_havelock;
  bool m_status;

  // lock based member functions
  bool createLock( void );
  bool openLock( void );
  bool closeLock( void );
  bool setLock( void );
};



#endif // _FILELOCK_H_
