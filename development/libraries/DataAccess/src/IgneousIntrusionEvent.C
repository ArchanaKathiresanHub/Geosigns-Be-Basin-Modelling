#include "Interface/IgneousIntrusionEvent.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/ProjectHandle.h"

#include <sstream>
#include <iostream>
using namespace std;

DataAccess::Interface::IgneousIntrusionEvent::IgneousIntrusionEvent ( ProjectHandle* projectHandle, const Formation* formation ) : DAObject ( projectHandle, 0 ) {

   m_formation = formation;
   m_snapshot = m_projectHandle->findSnapshot ( m_formation->getIgneousIntrusionAge ());

   // Intrusion event and snapshot must have the same age.
   assert ( m_formation->getIgneousIntrusionAge () == m_snapshot->getTime ());

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

double DataAccess::Interface::IgneousIntrusionEvent::getIntrusionTemperature () const {
   return m_formation->getIgneousIntrusionTemperature ();
}

std::string DataAccess::Interface::IgneousIntrusionEvent::image () const {

   std::stringstream buffer;

   buffer << "Igneous intrusion event." << std::endl;
   buffer << "  associated formation  : " << m_formation->getName () << std::endl;
   buffer << "  intrusion age         : " << getEndOfIntrusion ()->getTime () << std::endl;
   buffer << "  intrusion temperature : " << getIntrusionTemperature () << std::endl;

   return buffer.str ();
}
