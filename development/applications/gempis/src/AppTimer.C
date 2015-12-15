/*********************************************************************
*                                                                    
* Package:        AppTimer
*
* Dependencies:   Qt windowing system; 
*
*********************************************************************/

/// 
/// Provides a one-shot timer that can be used by an application. The 
/// advantage of a one-shot timer that it guraentees a delay period
/// instead of forcing an update every x seconds. This is useful when 
/// the duration of an action may take longer than your refresh period.
/// 

#include "includestreams.h"  // platform type definitions

// project include files
#include <AppTimer.h>

// Qt include files
#include <qapplication.h> 
#include <qtimer.h> 


///////////////////////////////////////////////////////////////////////
//
// c'tor
//
// Example:   m_appTimer = new AppTimer( this, SLOT(updateWindow()) );
//
// In this case updateWindow is defined as a public slots: in the 
// calling class.
//
AppTimer::AppTimer( QObject * receiver, char * member ) :
  m_receiver( receiver ),
  m_member( member ),
  m_duration( 1000 )
{
#ifdef DEBUG_APPTIMER
  cout << this << " AppTimer::AppTimer()" << endl;
#endif

  // define timer
  m_internalTimer = new QTimer( this );
}

///////////////////////////////////////////////////////////////////////
//
// d'tor
//
AppTimer::~AppTimer()
{
#ifdef DEBUG_APPTIMER
  cout << this << " AppTimer::~AppTimer()" << endl;
#endif

  // turn off timer
  Stop();

  delete m_internalTimer;
}


///////////////////////////////////////////////////////////////////////
//
// Sets the timer duration
//
void AppTimer::Set( long int duration )
{
#ifdef DEBUG_APPTIMER
  cout << this << " AppTimer::Set(" << duration << ")" << endl;
#endif

  m_duration = duration;
}


///////////////////////////////////////////////////////////////////////
//
// start timer
//
void AppTimer::Start( void )
{
#ifdef DEBUG_APPTIMER
  cout << this << " AppTimer::Start()" << endl;
#endif

  m_internalTimer->singleShot( m_duration, m_receiver, m_member );
}


///////////////////////////////////////////////////////////////////////
//
// stops timer
//
void AppTimer::Stop( void )
{
#ifdef DEBUG_APPTIMER
  cout << this << " AppTimer::Stop(()" << endl;
#endif

  // disable activation
  disconnect( m_internalTimer, SIGNAL(timeout()), m_receiver, SLOT(m_member()) );

  // turn timer off
  m_internalTimer->stop();
}


