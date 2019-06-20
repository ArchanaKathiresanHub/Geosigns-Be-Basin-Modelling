//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MINING__BASEMENT_HEAT_FLOW_CALCULATOR__H_
#define _MINING__BASEMENT_HEAT_FLOW_CALCULATOR__H_

#include "Snapshot.h"
#include "GridMap.h"
#include "Formation.h"
#include "Property.h"
#include "PropertyValue.h"

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

         AbstractDerivedProperties::FormationPropertyPtr m_temperature;
         AbstractDerivedProperties::FormationPropertyPtr m_pressure;
         AbstractDerivedProperties::FormationPropertyPtr m_depth;
         AbstractDerivedProperties::FormationPropertyPtr m_ves;
         AbstractDerivedProperties::FormationPropertyPtr m_maxVes;

         const GeoPhysics::GeoPhysicsFormation*   m_crust;
         const GeoPhysics::GeoPhysicsFormation*   m_bottomSediment;

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
