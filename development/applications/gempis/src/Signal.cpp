///////////////////////////////////////////////////////////////////////////////
// Signal.cpp - See Signal.h for a description on how to use
//
// Written by: Greg Lackore
//

#include <stdio.h>            // printf(), perror(), snprintf(), fopen(), ...
#include <stdlib.h>           // exit()
#include <errno.h>            // errno, EEXIST
#include <signal.h>           // sigaction

#include <iostream>
using namespace std;

#include "Signal.h"

// activating this will turn on routine messages
//#define DEBUG_SIGNAL
// this one will define a main for testing
//#define TEST_SIGNAL



///////////////////////////////////////////////////////////////////////////////
//
// Class public members
//


// c'tor  
//
Signal::Signal( void (* signalHandler)( int signum ) ) 
{
#if defined( DEBUG_SIGNAL )
  cout << "DEBUG(" << this << ") Signal::Signal()" << endl;
#endif

  // set up signal handlers
  defineHandlers( signalHandler );
}


// d'tor
//
Signal::~Signal()
{
#if defined( DEBUG_SIGNAL )
  cout << "DEBUG(" << this << ") Signal::~Signal()" << endl;
#endif

  // restore previous handlers
  restoreHandlers();
}


////////////////////////////////////////////////////////////////////////////////
//
// Class private members
//


// defines a new handler for all of the standard user signals
//
void Signal::defineHandlers( void (* signalHandler)( int signum ) )
{
#if defined( DEBUG_SIGNAL )
  cout << "DEBUG(" << this << ") Signal::defineHandlers()" << endl;
#endif

  // define handler for INTR  
  m_old_intr_action = (struct sigaction *)malloc( sizeof(struct sigaction) );
  defineHandler( SIGINT, signalHandler, m_old_intr_action );

  // define handler for QUIT
  m_old_quit_action = (struct sigaction *)malloc( sizeof(struct sigaction) );
  defineHandler( SIGQUIT, signalHandler, m_old_quit_action );

  // define handler for HUP
  m_old_hup_action = (struct sigaction *)malloc( sizeof(struct sigaction) );
  defineHandler( SIGHUP, signalHandler, m_old_hup_action );

  // define handler for TERM
  m_old_term_action = (struct sigaction *)malloc( sizeof(struct sigaction) );
  defineHandler( SIGTERM, signalHandler, m_old_term_action );
}


// this member function restores all user signal handlers to their original values
//
void Signal::restoreHandlers()
{
#if defined( DEBUG_SIGNAL )
  cout << "DEBUG(" << this << ") Signal::restoreHandlers()" << endl;
#endif

  // define handler for INTR     
  restoreHandler( SIGINT, m_old_intr_action );
  free( m_old_intr_action );

  // define handler for QUIT
  restoreHandler( SIGQUIT, m_old_quit_action );
  free( m_old_quit_action );

  // define handler for HUP
  restoreHandler( SIGHUP, m_old_hup_action );
  free( m_old_hup_action );

  // define handler for TERM
  restoreHandler( SIGTERM, m_old_term_action );
  free( m_old_term_action );
}


// this member function defines a new handler for a signal, 
// saving the old handler for later
//
void Signal::defineHandler( int signal, 
			    void (* signalHandler)( int signum ), 
			    struct sigaction * old_action )
{
#if defined( DEBUG_SIGNAL )
  cout << "DEBUG(" << this << ") Signal::defineHandler()" << endl;
#endif

  // Set up the structure to specify the new action.
  struct sigaction new_action;
  new_action.sa_handler = signalHandler;
  if ( sigemptyset( &new_action.sa_mask ) == -1 )
  {
    cout << "Signal::defineHandler - error in sigemptyset for signal " << signal << endl;
    exit( 1 );
  }
  new_action.sa_flags = 0;

  // get (and save) the current action
  sigaction( signal, NULL, old_action );

  // if we are not told to ignore it
  if ( old_action->sa_handler != SIG_IGN )
  {
    // define handler for signal
    sigaction( signal, &new_action, NULL );
  }
}


// restores a handler to it's old value for a signal
//
void Signal::restoreHandler( int signal, 
			     struct sigaction * old_action )
{
#if defined( DEBUG_SIGNAL )
  cout << "DEBUG(" << this << ") Signal::restoreHandler()" << endl;
#endif

  // if we are not told to ignore it
  if ( old_action->sa_handler != SIG_IGN )
  {
    // restore previous handler for signal
    sigaction( signal, old_action, NULL );
  }
}


#if defined( TEST_SIGNAL )
///////////////////////////////////////////////////////////////////////////////
//
// this is a test main so you only need to compile and link Signal.c
//

// Signal handler flag and routine
static bool m_signal_detected = false;

// routine activated when signal has been detected
static void mySignalHandler( int signum )
{
  m_signal_detected = true;

  cout << endl << endl << "mySignalHandler: signal detected = " << signum << endl << endl;
}


int main(int argc, char **argv) 
{
  Signal x( mySignalHandler );

  int res;

  cout << "sleep( 100 )"<< endl;

  res = sleep( 100 );

  cout << "sleep returned " << res << endl;
}
#endif
