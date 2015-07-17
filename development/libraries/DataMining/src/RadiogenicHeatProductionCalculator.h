#ifndef _MINING__RADIOGENIC_HEAT_PRODUCTION_CALCULATOR__H_
#define _MINING__RADIOGENIC_HEAT_PRODUCTION_CALCULATOR__H_

#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "DerivedPropertyManager.h"

#include "DomainFormationProperty.h"
#include "DomainPropertyCollection.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

namespace DataAccess {

   namespace Mining {

      class RadiogenicHeatProductionCalculator : public DomainFormationProperty {

      public :

         RadiogenicHeatProductionCalculator ( const DomainPropertyCollection*            collection,
                                              DerivedProperties::DerivedPropertyManager& propertyManager,
                                              const Interface::Snapshot*                 snapshot,
                                              const Interface::Property*                 property );

         double compute ( const ElementPosition& position ) const;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class RadiogenicHeatProductionCalculatorAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };

   }

}

#endif // _MINING__RADIOGENIC_HEAT_PRODUCTION_CALCULATOR__H_
