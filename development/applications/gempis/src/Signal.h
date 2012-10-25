#ifndef _SIGNAL_H_
#define _SIGNAL_H_
///////////////////////////////////////////////////////////////////////////////
// Signal - a class that disables the most common user signals. Once defined 
//          the signal handler remains in effect until the class is destroyed.
//          The application must determine if a signal has been detected during
//          the call to the signalHandler routine. 
//
// Written by: Greg Lackore
//
// Blocking system routines like sleep and semop will return control to the 
// application which must determine the cause of the return.. 
//
//


// forward declaration
struct sigaction;

// Signal class
//
class Signal {

public:
  Signal( void (* signalHandler)( int signum ) ); 
  ~Signal();

private:
  // control structures to restore the previously defined interrupt handlers
  struct sigaction * m_old_intr_action;
  struct sigaction * m_old_quit_action;
  struct sigaction * m_old_hup_action;
  struct sigaction * m_old_term_action;

  void defineHandlers( void (* signalHandler)( int signum ) );
  void restoreHandlers();
  void defineHandler( int signal, void (* signalHandler)( int signum ), struct sigaction * old_action );
  void restoreHandler( int signal, struct sigaction * old_action );
};


#endif // _SIGNAL_H_ 
