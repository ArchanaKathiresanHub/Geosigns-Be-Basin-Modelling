#include "Gempis.h"


Gempis* Gempis::s_instance = 0;


// dummy define
Gempis::Gempis()
{
  m_state = false;
}

// defines instance
Gempis* Gempis::instance()
{
  if ( s_instance == 0 )
  { 
    s_instance = new Gempis;
  }
  return s_instance;
}

// turns on message display
void Gempis::activate( void )
{
  m_state = true;
}

// turns off message display
void Gempis::deactivate( void )
{
  m_state = false;
}

void Gempis::send( const GempisMessageType id,
                   const string& message ) 
{
  char b_id[16];
  //  char buffer[256];

  // don't allow line to be displayed if not activated
  if ( ! m_state ) 
    {
      return;
    }
  // write message type
  switch( id ) 
    {
    case VERBOSE:
      strcpy( b_id, "VERBOSE" );
      break;
    case INFO:
      strcpy( b_id, "INFO" );
      break;
    case WARNING:
      strcpy( b_id, "WARNING" );
      break;
    case ERROR:
      strcpy( b_id, "ERROR" );
      break;
    case PERCENT:
      strcpy( b_id, "PERCENT" );
      break;
    case STATE:
      strcpy( b_id, "STATE" );
      break;
    case LOG:
      strcpy( b_id, "LOG" );
      break;
    default:
      break;
    }
  

  cout << "Gempis_" << b_id << " " << message << endl;


}
