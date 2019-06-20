#ifndef _MINING__FLUID_VELOCITY_CALCULATOR__H_
#define _MINING__FLUID_VELOCITY_CALCULATOR__H_

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

      class FluidVelocityCalculator : public DomainFormationProperty {

      public :

         enum RequiredCalculation { X, Y, Z, MAGNITUDE };

         FluidVelocityCalculator ( const DomainPropertyCollection*            collection,
                                   DerivedProperties::DerivedPropertyManager& propertyManager,
                                   const Interface::Snapshot*                 snapshot,
                                   const Interface::Property*                 property,
                                   const RequiredCalculation                  calculation );

         /// Initialise the fluid-velocity calculator by getting the fluid-velocity-x, -y, -z and the magnitude properties.
         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

          const DomainProperty* m_depth;
          const DomainProperty* m_hydrostaticPressure;
          const DomainProperty* m_overpressure;
          DomainProperty*       m_permeabilityH; 
          DomainProperty*       m_permeabilityN;
          DomainProperty*       m_porosity;
          const DomainProperty* m_pressure;
          const DomainProperty* m_temperature;

         RequiredCalculation   m_calculation;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class FluidVelocityCalculatorAllocator : public DomainPropertyAllocator {

      public :

         FluidVelocityCalculatorAllocator ( const FluidVelocityCalculator::RequiredCalculation calculation );

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      private :

         FluidVelocityCalculator::RequiredCalculation m_calculation;

      };


   }

}

#endif // _MINING__FLUID_VELOCITY_CALCULATOR__H_
