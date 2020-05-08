#include <math.h>
#include <algorithm>

#include "Snapshot.h"
#include "FaultFileReader.h"
#include "Faulting.h"
#include "FaultElementCalculator.h"
#include "FaultFileReaderFactory.h"
#include <sstream>
using std::ostringstream;

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

const std::string FaultStatusNames [] =
{
   "NoFault", "Seal","Pass","Waste","SealOil","PassOil"
};

//------------------------------------------------------------//

FaultEvent::FaultEvent (const double & age, const std::string & statusName)
  : m_FaultAge (&age)
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

const double * FaultEvent::getAge () const
{
   return m_FaultAge;
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
   o << faultEvent.getAge() << "  " << faultEvent.getStatusName () << endl;

   return o;
}

//------------------------------------------------------------//

OverpressureFaultEvent::OverpressureFaultEvent ( const double & age,
                                                 const std::string& faultLithology,
                                                 const bool         usedInOverpressure )
  : m_FaultAge ( &age ), m_faultLithologyName ( faultLithology ), m_usedInOverpressureCalculation ( usedInOverpressure )
{
}

//------------------------------------------------------------//

const double * OverpressureFaultEvent::getAge () const {
   return m_FaultAge;
}

//------------------------------------------------------------//

const std::string& OverpressureFaultEvent::getFaultLithologyName () const {
   return m_faultLithologyName;
}

//------------------------------------------------------------//

bool OverpressureFaultEvent::getUsedInOverpressureCalculation () const
{
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

size_t Fault::getNumberOfMigrationEvents () const {
   return m_events.size ();
}

void Fault::addEvent (const double & age , const std::string & status)
{
   FaultEvent newEvent (age, status);
   //FaultEventLessThan eventComparison;

   m_events.push_back (newEvent);

   ///
   /// Need to keep the sequence in correct order, that is: oldest first.
   ///
   std::sort(m_events.begin(), m_events.end(), [](const FaultEvent a, const FaultEvent& b){ return a.getAge() > b.getAge(); });
}

//------------------------------------------------------------//


void Fault::addOverpressureEvent (const double & age, const std::string & faultLithology, const bool usedInOverpressure )
{
   OverpressureFaultEvent newEvent (age, faultLithology, usedInOverpressure);

   m_overpressureEvents.push_back (newEvent);

   ///
   /// Need to keep the sequence in correct order, that is: oldest first.
   ///
   std::sort(m_overpressureEvents.begin(), m_overpressureEvents.end(), [](const OverpressureFaultEvent a, const OverpressureFaultEvent& b){ return a.getAge() > b.getAge(); });
}

//------------------------------------------------------------//

const PointSequence & Fault::getFaultLine () const
{
   return m_faultLine;
}

//------------------------------------------------------------//



FaultStatus Fault::getStatus (const double age) const
{
   FaultEventSequence::const_iterator feIter;
   for (feIter = m_events.begin (); feIter != m_events.end (); ++feIter)
   {
      if (* (*feIter).getAge () >= age)
         return (*feIter).getStatus ();
   }
   return NoFault;
}

const std::string & Fault::getStatusName (const Snapshot * snapshot) const
{
   return FaultStatusNames[getStatus (snapshot->getTime())];
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
