#ifndef _MINING__BRINE_VISCOSITY_CALCULATOR__H_
#define _MINING__BRINE_VISCOSITY_CALCULATOR__H_

#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "DomainFormationProperty.h"
#include "DomainPropertyCollection.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

namespace DataAccess {

   namespace Mining {

      class BrineViscosityCalculator : public DomainFormationProperty {

      public :

         using DomainFormationProperty::compute;

         BrineViscosityCalculator ( const DomainPropertyCollection* collection,
                                    const Interface::Snapshot*      snapshot,
                                    const Interface::Property*      property );

         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

         const DomainProperty* m_temperature;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class BrineViscosityCalculatorAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection* collection,
                                    const Interface::Snapshot*      snapshot,
                                    const Interface::Property*      property ) const;

      };


   }

}

#endif // _MINING__BRINE_VISCOSITY_CALCULATOR__H_
