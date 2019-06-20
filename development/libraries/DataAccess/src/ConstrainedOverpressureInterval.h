#ifndef _INTERFACE_CONSTRAINED_OVERPRESSURE_INTERVAL_H
#define _INTERFACE_CONSTRAINED_OVERPRESSURE_INTERVAL_H

#include <string>
using namespace std;

#include "DAObject.h"
#include "Interface.h"

namespace DataAccess
{
   namespace Interface
   {

      /// A ConstrainedOverpressureInterval provides an overpressure value for some interval of time.
      class ConstrainedOverpressureInterval : public DAObject
      {

      public :

         ConstrainedOverpressureInterval ( ProjectHandle*    projectHandle,
                                           database::Record* record,
                                           const Formation*  formation );

         virtual ~ConstrainedOverpressureInterval ();


         /// Return a constant pointer to the formation associated with this constraint overpressure interval.
         virtual const Formation* getFormation () const;

         /// Return a constant pointer to a snapshot for the start of the interval.
         virtual const Snapshot* getStartAge () const;

         /// Return a constant pointer to a snapshot for the end of the interval.
         virtual const Snapshot* getEndAge () const;

         /// Return the constrained overpressure value.
         virtual double getOverpressureValue () const;

      private :

         const Formation* m_formation;
         const Snapshot* m_startAge;
         const Snapshot* m_endAge;
         

      };
   }
}

#endif // _INTERFACE_CONSTRAINED_OVERPRESSURE_INTERVAL_H
