#include "Interface/IgneousIntrusionEvent.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/ProjectHandle.h"



#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include <sstream>
#include <iostream>
using namespace std;

DataAccess::Interface::IgneousIntrusionEvent::IgneousIntrusionEvent ( ProjectHandle* projectHandle, database::Record * record ) : DAObject ( projectHandle, record ) {

   double intrusionAge;

   m_formation = m_projectHandle->findFormation ( database::getLayerName (m_record));
   assert (("The formation for the igneous intrusion is invalid", m_formation != 0 ));
   
   intrusionAge = database::getIgneousIntrusionAge ( m_record );
   assert (("The intrusion age is no data value", intrusionAge != DefaultUndefinedScalarValue ));
   assert (( "The time of intrusion should be smaller than the time of deposition", intrusionAge <= m_formation->getTopSurface()->getSnapshot()->getTime() ));
       
   m_snapshot = m_projectHandle->findSnapshot ( intrusionAge );
   assert (("The snapshot is invalid", m_snapshot != 0 ));
   
   // Intrusion event and snapshot must have the same age.
   assert (("The intrusion age must have the same age than his snapshot", intrusionAge == m_snapshot->getTime ()));
   
}

DataAccess::Interface::IgneousIntrusionEvent::~IgneousIntrusionEvent () {
   m_formation = 0;
}

const DataAccess::Interface::Formation* DataAccess::Interface::IgneousIntrusionEvent::getFormation () const {
   return m_formation;
}

double DataAccess::Interface::IgneousIntrusionEvent::getStartOfIntrusion () const {
   return m_snapshot->getTime () + IgneousIntrusionEventDuration;
}

const DataAccess::Interface::Snapshot* DataAccess::Interface::IgneousIntrusionEvent::getEndOfIntrusion () const {
   return m_snapshot;
}


std::string DataAccess::Interface::IgneousIntrusionEvent::image () const {

   std::stringstream buffer;

   buffer << "Igneous intrusion event." << std::endl;
   buffer << "  associated formation  : " << m_formation->getName () << std::endl;
   buffer << "  intrusion start age   : " << getStartOfIntrusion () << std::endl;
   buffer << "  intrusion end age     : " << getEndOfIntrusion ()->getTime () << std::endl;

   return buffer.str ();
}
