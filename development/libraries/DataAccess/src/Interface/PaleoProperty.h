#ifndef _INTERFACE_PALEOPROPERTY_H_
#define _INTERFACE_PALEOPROPERTY_H_


#include "Interface/Interface.h"
#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {

      /// Contains a property in paleo-time.
      class PaleoProperty : public DAObject
      {
      public:

         PaleoProperty (ProjectHandle * projectHandle, database::Record * record);



         PaleoProperty ( ProjectHandle *      projectHandle,
                         const Formation*     formation,
                         const PaleoProperty* startProperty,
                         const PaleoProperty* endProperty,
                         const Snapshot*      interpolationTime );

         virtual ~PaleoProperty (void);


         /// Return the the Snapshot of this MobileLayer
         virtual const Snapshot * getSnapshot (void) const;

         /// Return the (GridMap) value of one of this PaleoProperties attributes
         /// If 0 is returned, no value was specified.
         virtual const GridMap * getMap (PaleoPropertyMapAttributeId attributeId) const;

         /// load a map
         GridMap * loadMap (PaleoPropertyMapAttributeId attributeId) const;

         /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary operator.
         GridMap * computeMap ( const PaleoPropertyMapAttributeId attributeId,
                                const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor) const;

         /// Print the attributes of this MobileLayer
         // May not work if user application is compiled under IRIX with CC -lang:std
         virtual void printOn (ostream &) const;

         virtual void asString (string &) const;

      protected:

         const Snapshot * m_snapshot;

         /// \brief Used only when interpolating paleo-properties.
         const PaleoProperty* m_startProperty;
         const PaleoProperty* m_endProperty;

         static const string s_MapAttributeNames[];

      };

      /// \brief Functor class for ordering Paleo properties
      class PaleoPropertyTimeLessThan {
         
      public :
         
         bool operator ()( const PaleoProperty* s1, 
                           const PaleoProperty* s2 ) const;
         
      };
    
   }
}

#endif // _INTERFACE_PALEOPROPERTY_H_
