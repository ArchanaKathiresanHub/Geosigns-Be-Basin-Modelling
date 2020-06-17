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

            FaultEvent (const double & age, const std::string & statusName);

            double getAge () const;
            FaultStatus getStatus () const;
            const std::string & getStatusName () const;

            friend std::ostream & operator<< (std::ostream & o, const FaultEvent & faultEvent);

         private:
            double m_faultAge;
            FaultStatus m_status;
      };


      //------------------------------------------------------------//


      class OverpressureFaultEvent {

      public :

         OverpressureFaultEvent ( const double & age,
                                  const std::string& faultLithology,
                                  const bool usedInOverpressure );

         double getAge() const;

         const std::string& getFaultLithologyName () const;

         bool getUsedInOverpressureCalculation () const;

      private :

         double          m_faultAge;
         std::string     m_faultLithologyName;
         bool            m_usedInOverpressureCalculation;

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

   virtual const std::string & getName (void) const;

         ///
         ///
         ///
         void addEvent (const double & age, const std::string & status);

         void addOverpressureEvent (const double & age,
                                    const std::string& faultLithology,
                                    const bool         usedInOverpressure );

         const PointSequence & getFaultLine () const;

         virtual PointList * getPoints () const;

         FaultStatus getStatus (double age) const;

         /// \brief Determine the number of migration events a fault has.
         size_t getNumberOfMigrationEvents () const;

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

         friend std::ostream & operator<< (std::ostream & o, const Fault & singleFault);
      };
   }
}

#endif // _INTERFACE_FAULTING_H_
