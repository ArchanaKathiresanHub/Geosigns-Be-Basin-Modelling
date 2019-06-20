// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_PALEOFORMATIONPROPERTY_H_
#define _INTERFACE_PALEOFORMATIONPROPERTY_H_


#include "Interface.h"
#include "PaleoProperty.h"

namespace DataAccess
{
   namespace Interface
   {

      /// Contains a property for a formation in paleo-time.
      class PaleoFormationProperty : public PaleoProperty 
      {
      public:

         PaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const Formation* formation );

         PaleoFormationProperty ( ProjectHandle *               projectHandle,
                                  const Formation*              formation,
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

      protected:

         const Formation * m_formation;
         const string formationName;

      };

   }
}

#endif // _INTERFACE_PALEOFORMATIONPROPERTY_H_
