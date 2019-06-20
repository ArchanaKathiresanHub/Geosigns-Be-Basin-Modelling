#ifndef _MINING__THERMAL_CONDUCTIVITY_CALCULATOR__H_
#define _MINING__THERMAL_CONDUCTIVITY_CALCULATOR__H_

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

      class ThermalConductivityCalculator : public DomainFormationProperty {

      public :

         ThermalConductivityCalculator ( const DomainPropertyCollection*            collection,
                                         DerivedProperties::DerivedPropertyManager& propertyManager,
                                         const Interface::Snapshot*                 snapshot,
                                         const Interface::Property*                 property,
                                         const bool                                 normalConductivity );

         /// Initialise the permeability-calculator by getting the ves and max-ves properties.
         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

         const DomainProperty* m_temperature;
         const DomainProperty* m_pressure;
         DomainProperty*       m_porosity;
         bool                  m_normalConductivity;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class ThermalConductivityCalculatorAllocator : public DomainPropertyAllocator {

      public :

         ThermalConductivityCalculatorAllocator ( const bool normalConductivity );

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;


      private :

         bool m_normalConductivity;

      };

   }

}

#endif // _MINING__THERMAL_CONDUCTIVITY_CALCULATOR__H_
