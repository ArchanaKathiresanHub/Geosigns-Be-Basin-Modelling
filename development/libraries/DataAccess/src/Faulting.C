#include <math.h>
#include <algorithm>

#include "Interface/Snapshot.h"
#include "Interface/FaultFileReader.h"
#include "Interface/Faulting.h"
#include "Interface/FaultElementCalculator.h"
#include "Interface/FaultFileReaderFactory.h"

#ifdef sgi
#ifdef _STANDARD_C_PLUS_PLUS
#include<sstream>
using std::ostringstream;
#else                           // !_STANDARD_C_PLUS_PLUS
#include<strstream.h>
typedef strstream ostringstream;
#endif                          // _STANDARD_C_PLUS_PLUS
#else                           // !sgi
#include <sstream>
using std::ostringstream;
#endif                          // sgi

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

const std::string FaultStatusNames [] =
{
   "NoFault", "Seal","Pass","Waste","SealOil","PassOil"
};

//------------------------------------------------------------//

FaultEvent::FaultEvent (const Snapshot * snapshot, const std::string & statusName) : m_snapshot (snapshot)
{
   m_status = NoFault;

   for (int i = 1; i !=  (int) NumFaults; ++i)
   {
      if (statusName == FaultStatusNames[i])
      {
         m_status = (FaultStatus) i;
      }
   }
}

//------------------------------------------------------------//

const Snapshot * FaultEvent::getSnapshot () const
{
   return m_snapshot;
}

//------------------------------------------------------------//

FaultStatus FaultEvent::getStatus () const
{
   return m_status;
}

//------------------------------------------------------------//

const std::string & FaultEvent::getStatusName () const
{
   return FaultStatusNames[m_status];
}


//------------------------------------------------------------//

ostream & DataAccess::Interface::operator<< (ostream & o, const FaultEvent & faultEvent)
{
   o << faultEvent.getSnapshot ()->getTime () << "  " << faultEvent.getStatusName () << endl;

   return o;
}

//------------------------------------------------------------//

bool FaultEventLessThan::operator  () (const FaultEvent & event1, const FaultEvent & event2)
     const
     {

        return * event1.getSnapshot () < * event2.getSnapshot ();
     }

//------------------------------------------------------------//

bool OverpressureFaultEventLessThan::operator () (const OverpressureFaultEvent & event1, const OverpressureFaultEvent & event2) const
{
   return * event1.getSnapshot () < * event2.getSnapshot ();
}

//------------------------------------------------------------//

OverpressureFaultEvent::OverpressureFaultEvent ( const Snapshot*    snapshot,
                                                 const std::string& faultLithology,
                                                 const bool         usedInOverpressure ) :
   m_snapshot ( snapshot ), m_faultLithologyName ( faultLithology ), m_usedInOverpressureCalculation ( usedInOverpressure ) {
   
}

//------------------------------------------------------------//

const Snapshot * OverpressureFaultEvent::getSnapshot () const {
   return m_snapshot;
}

//------------------------------------------------------------//

const std::string& OverpressureFaultEvent::getFaultLithologyName () const {
   return m_faultLithologyName;
}

//------------------------------------------------------------//

bool OverpressureFaultEvent::getUsedInOverpressureCalculation () const {
   return m_usedInOverpressureCalculation;
}

//------------------------------------------------------------//

Fault::Fault (const std::string & Name, const PointSequence & Line)
{
   m_faultName = Name;
   m_faultLine = Line;
}

//------------------------------------------------------------//
Fault::~Fault ()
{
   m_faultLine.clear ();
   m_events.clear ();
}

const string & Fault::getName (void) const
{
   return m_faultName;
}

PointList * Fault::getPoints (void) const
{
   PointSequence::const_iterator pointIter;

   PointList * points = new PointList;

   for (pointIter = m_faultLine.begin (); pointIter != m_faultLine.end (); ++pointIter)
   {
      points->push_back (&(*pointIter));
   }

   return points;
}

void Fault::addEvent (const Snapshot * snapshot, const std::string & status)
{
   FaultEvent newEvent (snapshot, status);
   FaultEventLessThan eventComparison;

   m_events.push_back (newEvent);

   ///
   /// Need to keep the sequence in correct order, that is: oldest first.
   ///
   std::sort (m_events.begin (), m_events.end (), eventComparison);
}

//------------------------------------------------------------//


void Fault::addOverpressureEvent (const Snapshot * snapshot, const std::string & faultLithology, const bool usedInOverpressure )
{
   OverpressureFaultEvent newEvent (snapshot, faultLithology, usedInOverpressure );
   OverpressureFaultEventLessThan eventComparison;

   m_overpressureEvents.push_back (newEvent);

   ///
   /// Need to keep the sequence in correct order, that is: oldest first.
   ///
   std::sort ( m_overpressureEvents.begin (), m_overpressureEvents.end (), eventComparison);
}

//------------------------------------------------------------//

const PointSequence & Fault::getFaultLine () const
{
   return m_faultLine;
}

//------------------------------------------------------------//



FaultStatus Fault::getStatus (const Snapshot * snapshot) const
{
   FaultEventSequence::const_iterator feIter;
   for (feIter = m_events.begin (); feIter != m_events.end (); ++feIter)
   {
      if (* (*feIter).getSnapshot () >= * snapshot)
         return (*feIter).getStatus ();
   }
   return NoFault;
}

const std::string & Fault::getStatusName (const Snapshot * snapshot) const
{
   return FaultStatusNames[getStatus (snapshot)];
}


Fault::OverpressureFaultEventIterator Fault::beginOverpressureEvents () const {
   return m_overpressureEvents.begin ();
}

Fault::OverpressureFaultEventIterator Fault::endOverpressureEvents () const {
   return m_overpressureEvents.end ();
}

//------------------------------------------------------------//

ostream & DataAccess::Interface::operator<< (ostream & o, const Fault & singleFault)
{
   Fault::FaultEventSequence::const_iterator event;

   o << "fault name: " << singleFault.m_faultName << endl;
   o << "points (" << singleFault.m_faultLine.size () << "):" << endl;

   size_t i;

   for (i = 0; i < singleFault.m_faultLine.size (); i++)
   {
      o << singleFault.m_faultLine[i];
   }

   o << "events (" << singleFault.m_events.size () << "):" << endl;
   for (event = singleFault.m_events.begin (); event != singleFault.m_events.end (); ++event)
   {
      o << *event;
   }

   return o;
}
