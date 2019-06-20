#ifndef _MINING__RADIOGENIC_HEAT_PRODUCTION_CALCULATOR__H_
#define _MINING__RADIOGENIC_HEAT_PRODUCTION_CALCULATOR__H_

#include "Snapshot.h"
#include "GridMap.h"
#include "Formation.h"
#include "Property.h"
#include "PropertyValue.h"

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
