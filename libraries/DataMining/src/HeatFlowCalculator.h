#ifndef _MINING__HEAT_FLOW_CALCULATOR__H_
#define _MINING__HEAT_FLOW_CALCULATOR__H_

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

      class HeatFlowCalculator : public DomainFormationProperty {

      public :

         enum RequiredCalculation { X, Y, Z, MAGNITUDE };

         HeatFlowCalculator ( const DomainPropertyCollection*  collection,
                              const Interface::Snapshot* snapshot,
                              const Interface::Property* property,
                              const RequiredCalculation  calculation );

         /// Initialise the heat-flow calculator by getting the heat-flow-x, -y, -z and the magnitude properties.
         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

         FormationToPropertyValueMapping m_depthValues;

         const DomainProperty* m_ves;
         const DomainProperty* m_maxVes;

         const DomainFormationProperty* m_depth;
         const DomainFormationProperty* m_temperature;
         const DomainFormationProperty* m_overpressure;
         const DomainFormationProperty* m_hydrostaticPressure;

         RequiredCalculation   m_calculation;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class HeatFlowCalculatorAllocator : public DomainPropertyAllocator {

      public :

         HeatFlowCalculatorAllocator ( const HeatFlowCalculator::RequiredCalculation calculation );

         DomainProperty* allocate ( const DomainPropertyCollection*  collection,
                                    const Interface::Snapshot* snapshot,
                                    const Interface::Property* property ) const;

      private :

         HeatFlowCalculator::RequiredCalculation m_calculation;

      };


   }

}

#endif // _MINING__HEAT_FLOW_CALCULATOR__H_
