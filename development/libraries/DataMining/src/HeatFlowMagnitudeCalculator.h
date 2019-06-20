#ifndef _MINING__HEAT_FLOW_MAGNITUDE_CALCULATOR__H_
#define _MINING__HEAT_FLOW_MAGNITUDE_CALCULATOR__H_

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

      class HeatFlowMagnitudeCalculator : public DomainFormationProperty {

      public :

         HeatFlowMagnitudeCalculator ( const DomainPropertyCollection*            collection,
                                       DerivedProperties::DerivedPropertyManager& propertyManager,
                                       const Interface::Snapshot*                 snapshot,
                                       const Interface::Property*                 property );

         /// Initialise the heat-flow-magniture calculator by getting the heat-flow-x, -y and -z properties.
         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

         const DomainProperty* m_heatFlowX;
         const DomainProperty* m_heatFlowY;
         const DomainProperty* m_heatFlowZ;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class HeatFlowMagnitudeCalculatorAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };


   }

}

#endif // _MINING__HEAT_FLOW_MAGNITUDE_CALCULATOR__H_
