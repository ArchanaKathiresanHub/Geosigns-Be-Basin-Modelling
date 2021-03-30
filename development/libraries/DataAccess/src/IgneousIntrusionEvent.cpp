#include "IgneousIntrusionEvent.h"
#include "Formation.h"
#include "Snapshot.h"
#include "Surface.h"
#include "ProjectHandle.h"



#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include <sstream>
#include <iostream>
using namespace std;

DataAccess::Interface::IgneousIntrusionEvent::IgneousIntrusionEvent ( ProjectHandle& projectHandle, database::Record * record ) : DAObject ( projectHandle, record ) {

   double intrusionAge;

   m_formation = projectHandle.findFormation ( database::getLayerName (m_record));
   assert ( m_formation != nullptr );

   intrusionAge = database::getIgneousIntrusionAge ( m_record );
   assert ( intrusionAge != DefaultUndefinedScalarValue );
   assert ( intrusionAge <= m_formation->getTopSurface()->getSnapshot()->getTime() );

   m_snapshot = projectHandle.findSnapshot ( intrusionAge );
   assert ( m_snapshot != nullptr );

   // Intrusion event and snapshot must have the same age.
   assert ( intrusionAge == m_snapshot->getTime() );

}

DataAccess::Interface::IgneousIntrusionEvent::~IgneousIntrusionEvent () {
   m_formation = nullptr;
}

const DataAccess::Interface::Formation* DataAccess::Interface::IgneousIntrusionEvent::getFormation () const {
   return m_formation;
}

double DataAccess::Interface::IgneousIntrusionEvent::getStartOfIntrusion () const {
   return m_snapshot->getTime () + IgneousIntrusionEventDuration;
}

double DataAccess::Interface::IgneousIntrusionEvent::getEndOfIntrusion () const {
   return m_snapshot->getTime ();
}


std::string DataAccess::Interface::IgneousIntrusionEvent::image () const {

   std::stringstream buffer;

   buffer << "Igneous intrusion event." << std::endl;
   buffer << "  associated formation  : " << m_formation->getName () << std::endl;
   buffer << "  intrusion start age   : " << getStartOfIntrusion () << std::endl;
   buffer << "  intrusion end age     : " << getEndOfIntrusion () << std::endl;

   return buffer.str ();
}
