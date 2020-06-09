#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "ConstrainedOverpressureInterval.h"

#include "Snapshot.h"
#include "Formation.h"
#include "ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;


ConstrainedOverpressureInterval::ConstrainedOverpressureInterval ( ProjectHandle& projectHandle,
                                                                   Record * record,
                                                                   const Formation*  formation) :
   DAObject (projectHandle, record) {

   m_formation = formation;
   m_startAge = projectHandle.findSnapshot ( database::getBeginTimeValues ( m_record ));
   m_endAge = projectHandle.findSnapshot ( database::getEndTimeValues ( m_record ));

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
