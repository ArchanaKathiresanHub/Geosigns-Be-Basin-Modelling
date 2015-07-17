#ifndef _MINING__BRINE_DENSITY_CALCULATOR__H_
#define _MINING__BRINE_DENSITY_CALCULATOR__H_

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

      class BrineDensityCalculator : public DomainFormationProperty {

      public :

         using DomainFormationProperty::compute;

         BrineDensityCalculator ( const DomainPropertyCollection*            collection,
                                  DerivedProperties::DerivedPropertyManager& propertyManager,
                                  const Interface::Snapshot*                 snapshot,
                                  const Interface::Property*                 property );

         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

         const DomainProperty* m_temperature;
         const DomainProperty* m_porePressure;

         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class BrineDensityCalculatorAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };


   }

}

#endif // _MINING__BRINE_DENSITY_CALCULATOR__H_
