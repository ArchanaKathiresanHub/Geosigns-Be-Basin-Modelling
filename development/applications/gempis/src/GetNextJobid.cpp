///////////////////////////////////////////////////////////////////////////////
// GetNextJobId.cpp - Get the next Gempis job id
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
#include "Signal.h"
#include "JobId.h"

// when active will loop infinitly
//#define DEBUG_GETNEXTJOBID


// show message in verbose mode
bool verbose_flag = false;
void verbose( const char * string )
{
  if ( verbose_flag == true )
  {
    cout << string << endl;
  }
}

// terminates application after displaying a usage message
void usage( const char * msg )
{
  verbose( msg );
  verbose( "Usage: getnextjobid -v -t <filename> <lock_filename>" );
  exit( 1 );
}

// signal handler routine to prevent application from terminating in an 
// unknown state.
//
bool termination_detected = false;
void terminate_loop (int signum)
{
  //cerr << endl << endl << "GetNextJobid: Signal " << signum << " detected" << endl << endl;
  termination_detected = true;
}


//
// main routine
//
// Returns: successful
//          Otherwise, error
bool test_flag = false;   // == test file locking looping until user terminates
extern char *optarg;
extern int optind, opterr;
int main( int argc, char **argv ) 
{
  // filenames
  string lock_filename;
  string data_filename;
  string str;

  // parse parameters
  int c;
  int count = 0;
  int result = 0;
  static char optstring[] = "-vt";
  opterr = 0;
  while ( (c = getopt(argc, argv, optstring)) != -1 )
  {
    switch (c) 
    {
    case 1:
      switch ( count++ )
      {
      case 0:
        verbose( "Setting data_filename." );
        data_filename = optarg;
        break;
      case 1:
        verbose( "Setting lock_filename." );
        lock_filename = optarg;
        break;
      default:
        char buf[1024];
        sprintf( buf, "Error unknown option [%s]", optarg );
        usage( buf );  // won't return
      }
      break;
    case 'v':
      verbose_flag = true;
      cout << "Verbose on." << endl;
      break;
    case 't':
      test_flag = true;
      cout << "Run in test mode." << endl;
      break;
    case '?':
      char buf[1024];
      sprintf( buf, "Error unknown option [%s]", optarg );
      usage( buf );  // won't return
    }
  }

  /*
  cout << lock_filename << endl;
  cout << data_filename << endl;
  */

  // check values
  if ( lock_filename.length() == 0 )
  {
    usage( "lock_filename not defined." );
  }
  if ( data_filename.length() == 0 )
  {
    usage( "data_filename not defined." );
  }

  // set files to be used
  if ( JobId::Instance()->SetFileNames(data_filename, lock_filename) == false )
  {
    verbose( "Error setting file names." );
    exit( 1 );
  }
	     
  // run 
  if ( test_flag == false )
  {
    // if not a test once 
    if ( JobId::Instance()->SetNext() == false )
    {
      verbose( "Error detected setting next jobid." );
      exit( 1 );
    }
    cout << JobId::Instance()->Get() << endl;
  }
  else
  {
    // define signal handler, deletes itself when complete
    Signal x( terminate_loop );

    // this is a loop to test file locking 
    int i=0;
    while ( termination_detected == false )
    {
      // display a loop counter
      char buf[1024];
      sprintf( buf, "Loop %d", i++ );
      verbose( buf );

      if ( JobId::Instance()->SetNext() == false )
      {
        verbose( "Error detected setting next jobid. Terminating test." );
        exit( 1 );
      }
      cout << JobId::Instance()->Get() << endl;
    }
    verbose( "Test completed..." );
  }
  exit( 0 );
}
