#ifndef _GEMPIS_H_
#define _GEMPIS_H_

#include <string>

#include <iostream>
#include <assert.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
using namespace std;

#include <string.h>

//#include <petscksp.h>
//#include "petscts.h"
//#include "petscda.h"
//#include "petscvector_readwrite.h"
 

enum GempisMessageType {
  VERBOSE,
  INFO,
  WARNING,
  ERROR,
  PERCENT,
  STATE,
  LOG
};

class Gempis {

public:
  static Gempis* instance();

  void activate( void );

  void deactivate( void );

  void send( const GempisMessageType id,
	     const string& message );

protected:
  Gempis();

private:
  static Gempis* s_instance;
  bool m_state;
};


#endif /* _GEMPIS_H_ */
