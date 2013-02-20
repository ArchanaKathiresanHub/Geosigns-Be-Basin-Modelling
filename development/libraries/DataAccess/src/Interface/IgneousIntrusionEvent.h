#ifndef _DATA_ACCESS__IGNEOUS_INTRUSION_EVENT__H_
#define _DATA_ACCESS__IGNEOUS_INTRUSION_EVENT__H_

using namespace std;

#include <string>

#include "Interface/DAObject.h"
#include "Interface/Interface.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"

namespace DataAccess
{

   namespace Interface
   {

      /// \brief A IgneousIntrusionEvent object contains information about a specific igneous intrusion event.
      class IgneousIntrusionEvent : public DAObject
      {
      public:

         IgneousIntrusionEvent ( ProjectHandle* projectHandle, const Formation* formation );

         virtual ~IgneousIntrusionEvent ();

         /// \brief Get the formation to which this intrusion event pertains.
         virtual const Formation* getFormation () const;

         /// \brief Get the age at which the intrusion starts to inflate.
         // Replace return type with a snapshot if we decide to have a snapshot 
         // at an age immediatly before the inflation.
         virtual double getStartOfIntrusion () const;

         /// \brief Get the age at which the intrusion finishes to inflate.
         virtual const Snapshot* getEndOfIntrusion () const;

         /// \brief Get the temperature at which the material is intruded.
         virtual double getIntrusionTemperature () const;

         virtual std::string image () const;

      private :

         const Formation* m_formation;
         const Snapshot*  m_snapshot;

      };

   }

}


#endif // _DATA_ACCESS__IGNEOUS_INTRUSION_EVENT__H_
