#ifndef _MINING__THERMAL_CONDUCTIVITY_CALCULATOR__H_
#define _MINING__THERMAL_CONDUCTIVITY_CALCULATOR__H_

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

      class ThermalConductivityCalculator : public DomainFormationProperty {

      public :

         ThermalConductivityCalculator ( const DomainPropertyCollection*  collection,
                                         const Interface::Snapshot* snapshot,
                                         const Interface::Property* property,
                                         const bool                 normalConductivity );

         /// Initialise the permeability-calculator by getting the ves and max-ves properties.
         bool initialise ();

         double compute ( const ElementPosition& position ) const;

      private :

         const DomainProperty* m_temperature;
         const DomainProperty* m_porosity;
         bool                  m_normalConductivity;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class ThermalConductivityCalculatorAllocator : public DomainPropertyAllocator {

      public :

         ThermalConductivityCalculatorAllocator ( const bool normalConductivity );

         DomainProperty* allocate ( const DomainPropertyCollection*  collection,
                                    const Interface::Snapshot* snapshot,
                                    const Interface::Property* property ) const;


      private :

         bool m_normalConductivity;

      };

   }

}

#endif // _MINING__THERMAL_CONDUCTIVITY_CALCULATOR__H_
