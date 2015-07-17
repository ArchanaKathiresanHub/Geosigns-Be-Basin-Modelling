#ifndef _MINING__BASEMENT_HEAT_FLOW_CALCULATOR__H_
#define _MINING__BASEMENT_HEAT_FLOW_CALCULATOR__H_

#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "GeoPhysicsFormation.h"

#include "DerivedPropertyManager.h"

#include "DomainFormationProperty.h"
#include "DomainPropertyCollection.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

namespace DataAccess {

   namespace Mining {

      class BasementHeatFlowCalculator : public DomainFormationProperty {

      public :

         using DomainFormationProperty::compute;

         BasementHeatFlowCalculator ( const DomainPropertyCollection*            collection,
                                      DerivedProperties::DerivedPropertyManager& propertyManager,
                                      const Interface::Snapshot*                 snapshot,
                                      const Interface::Property*                 property );

         /// Initialise the heat-flow calculator by getting the heat-flow-x, -y, -z and the magnitude properties.
         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

         DerivedProperties::FormationPropertyPtr m_temperature;
         DerivedProperties::FormationPropertyPtr m_pressure;
         DerivedProperties::FormationPropertyPtr m_depth;
         DerivedProperties::FormationPropertyPtr m_ves;
         DerivedProperties::FormationPropertyPtr m_maxVes;

         // const Interface::PropertyValue* m_temperature;
         // const Interface::PropertyValue* m_pressure;
         // const Interface::PropertyValue* m_depth;
         // const Interface::PropertyValue* m_ves;
         // const Interface::PropertyValue* m_maxVes;

         // const DomainProperty* m_ves;
         // const DomainProperty* m_maxVes;

         const GeoPhysics::Formation*   m_crust;
         const GeoPhysics::Formation*   m_bottomSediment;


         bool                  m_fromCrust;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class BasementHeatFlowCalculatorAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };


   }

}

#endif // _MINING__BASEMENT_HEAT_FLOW_CALCULATOR__H_
