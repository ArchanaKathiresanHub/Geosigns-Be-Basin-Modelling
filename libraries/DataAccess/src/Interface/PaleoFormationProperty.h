#ifndef _INTERFACE_PALEOFORMATIONPROPERTY_H_
#define _INTERFACE_PALEOFORMATIONPROPERTY_H_


#include "Interface/Interface.h"
#include "Interface/PaleoProperty.h"

namespace DataAccess
{
   namespace Interface
   {

      /// Contains a property for a formation in paleo-time.
      class PaleoFormationProperty : public PaleoProperty 
      {
      public:

#if 0
         PaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const string& initialName );
#endif

         PaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const Formation* formation );

         PaleoFormationProperty ( ProjectHandle * projectHandle,
                                  const Formation* formation,
                                  const PaleoFormationProperty* startProperty,
                                  const PaleoFormationProperty* endProperty,
                                  const Snapshot*               interpolationTime );



         ~PaleoFormationProperty (void);

         /// Return the Formation of this PaleoFormationProperty.
         const Formation * getFormation (void) const;

         /// Return the Formation name of this PaleoFormationProperty.
         const string & getFormationName (void) const;

         /// Return the string representation of the PaleoFormationProperty object.
         void asString (string &) const;

         //void setFormation ( const Formation* formation );


      protected:

         const Formation * m_formation;
         const string formationName;

      };

   }
}

#endif // _INTERFACE_PALEOFORMATIONPROPERTY_H_
