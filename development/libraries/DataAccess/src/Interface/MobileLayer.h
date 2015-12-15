#ifndef _INTERFACE_MOBILELAYER_H_
#define _INTERFACE_MOBILELAYER_H_

#include "Interface/Interface.h"
#include "Interface/DAObject.h"
#include "Interface/PaleoFormationProperty.h"

namespace DataAccess
{
   namespace Interface
   {
      /// MobileLayer Class
      class MobileLayer : public PaleoFormationProperty
      {
      public:

         MobileLayer (ProjectHandle * projectHandle, database::Record * record);
         ~MobileLayer (void);

         /// Return the Formation of this PaleoFormationProperty.
         //  Should this function be here? Should it be implemented in the PaleoFormationProperty?
         const Formation * getFormation (void) const;

	 virtual const GridMap * getMap (PaleoPropertyMapAttributeId attributeId) const;


         const std::string& getFormationName () const;

         void asString (string &) const;

      };
   }
}

#endif // _INTERFACE_MOBILELAYER_H_
