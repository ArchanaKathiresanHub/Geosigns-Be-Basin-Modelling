#include <assert.h>
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ConstrainedOverpressureInterval.h"

#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;


ConstrainedOverpressureInterval::ConstrainedOverpressureInterval ( ProjectHandle * projectHandle,
                                                                   Record * record,
                                                                   const Formation*  formation) : 
   DAObject (projectHandle, record) {

   m_formation = formation;
   m_startAge = m_projectHandle->findSnapshot ( database::getBeginTimeValues ( m_record ));
   m_endAge = m_projectHandle->findSnapshot ( database::getEndTimeValues ( m_record ));

}

ConstrainedOverpressureInterval::~ConstrainedOverpressureInterval (void)
{
}

const Formation* ConstrainedOverpressureInterval::getFormation () const {
   return m_formation;
}

const Snapshot* ConstrainedOverpressureInterval::getStartAge () const {
   return m_startAge;
}

const Snapshot* ConstrainedOverpressureInterval::getEndAge () const {
   return m_endAge;
}

double ConstrainedOverpressureInterval::getOverpressureValue () const {
   return database::getPropertyValue ( m_record );
}
