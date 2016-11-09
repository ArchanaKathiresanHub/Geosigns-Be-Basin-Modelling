#ifndef _INTERFACE_FAULTING_H_
#define _INTERFACE_FAULTING_H_

#include <iostream>
#include<fstream>
#include <vector>
#include <string>
#include <map>

#include "auxiliaryfaulttypes.h"
#include "Interface.h"

// #include "layer_iterators.h"

//------------------------------------------------------------//

namespace DataAccess
{
   namespace Interface
   {
      class Snapshot;

      class FaultEvent
      {

         public:

            FaultEvent (const Snapshot * snapshot, const std::string & statusName);

            const Snapshot * getSnapshot () const;
            FaultStatus getStatus () const;
            const std::string & getStatusName () const;

            friend ostream & operator<< (ostream & o, const FaultEvent & faultEvent);

         private:
            const Snapshot * m_snapshot;
            FaultStatus m_status;
      };

      /// Enables the overpressure-fault-events to be sorted (by age).
      class FaultEventLessThan
      {
         public:
            bool operator  () (const FaultEvent & event1, const FaultEvent & event2) const;
      };


      //------------------------------------------------------------//


      class OverpressureFaultEvent {

      public :

         OverpressureFaultEvent ( const Snapshot*    snapshot,
                                  const std::string& faultLithology,
                                  const bool         usedInOverpressure );

         const Snapshot * getSnapshot () const;

         const std::string& getFaultLithologyName () const;

         bool getUsedInOverpressureCalculation () const;

      private :

         const Snapshot* m_snapshot;
         std::string     m_faultLithologyName;
         bool            m_usedInOverpressureCalculation;

      };


      /// Enables the overpressure-fault-events to be sorted (by age).
      class OverpressureFaultEventLessThan
      {
      public:
         bool operator () (const OverpressureFaultEvent & event1, const OverpressureFaultEvent & event2) const;
      };

      //------------------------------------------------------------//


      ///
      /// An individual fault, containing a sequence of points (making up the geometry of the fault),
      /// and a sequence of events.
      ///
      class Fault
      {
         typedef std::vector < FaultEvent > FaultEventSequence;

         typedef std::vector < OverpressureFaultEvent > OverpressureFaultEventSequence;

      public:


         typedef OverpressureFaultEventSequence::const_iterator OverpressureFaultEventIterator;


         Fault (const std::string & Name, const PointSequence & Line);

         virtual ~Fault ();

	 virtual const string & getName (void) const;

         ///
         ///
         ///
         void addEvent (const Snapshot * snapshot, const std::string & status);

         void addOverpressureEvent (const Snapshot * snapshot,
                                    const std::string& faultLithology,
                                    const bool         usedInOverpressure );

         const PointSequence & getFaultLine () const;

	 virtual PointList * getPoints () const;

         FaultStatus getStatus (const Snapshot * snapshot) const;
         const std::string & getStatusName (const Snapshot * snapshot) const;

         OverpressureFaultEventIterator beginOverpressureEvents () const;

         OverpressureFaultEventIterator endOverpressureEvents () const;


      private:
         std::string m_faultName;

         /// The sequence of migration-events that the fault undergoes (sorted in time order, oldest first)
         FaultEventSequence m_events;

         /// The sequence of overpressure-events that the fault undergoes (sorted in time order, oldest first)
         OverpressureFaultEventSequence m_overpressureEvents;

         ///
         /// The geometry of the fault.
         ///
         PointSequence m_faultLine;

         friend ostream & operator<< (ostream & o, const Fault & singleFault);
      };
   }
}

#endif // _INTERFACE_FAULTING_H_
