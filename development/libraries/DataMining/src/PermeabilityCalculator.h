#ifndef _MINING__DOMAIN_PERMEABILITY_CALCULATOR__H_
#define _MINING__DOMAIN_PERMEABILITY_CALCULATOR__H_

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

      class PermeabilityCalculator : public DomainFormationProperty {

      public :

         PermeabilityCalculator ( const DomainPropertyCollection*            collection,
                                  DerivedProperties::DerivedPropertyManager& propertyManager,
                                  const Interface::Snapshot*                 snapshot,
                                  const Interface::Property*                 property,
                                  const bool                                 normalPermeability );

         /// Initialise the permeability-calculator by getting the ves and max-ves properties.
         bool initialise ();

         // /// Add the property, evaluated at the position, to the evaluations mapping.
         // ///
         // /// If the formation is null then a null-value will be added.
         // void compute ( const ElementPosition&            position,
         //                      InterpolatedPropertyValues& evaluations ) const;

         double compute ( const ElementPosition& position ) const;

      private :

         const DomainProperty* m_ves;
         const DomainProperty* m_maxVes;
         const DomainProperty* m_chemicalCompaction;
         bool                  m_chemicalCompactionRequired;
         bool                  m_normalPermeability;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class PermeabilityCalculatorAllocator : public DomainPropertyAllocator {

      public :

         PermeabilityCalculatorAllocator ( const bool normalPermeability );

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;


      private :

         bool m_normalPermeability;

      };

   }

}

#endif // _MINING__DOMAIN_PERMEABILITY_CALCULATOR__H_
