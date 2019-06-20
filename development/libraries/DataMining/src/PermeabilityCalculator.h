#ifndef _MINING__DOMAIN_PERMEABILITY_CALCULATOR__H_
#define _MINING__DOMAIN_PERMEABILITY_CALCULATOR__H_

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

      class PermeabilityCalculator : public DomainFormationProperty {

      public :

         using DomainFormationProperty::compute;

         PermeabilityCalculator ( const DomainPropertyCollection*            collection,
                                  DerivedProperties::DerivedPropertyManager& propertyManager,
                                  const Interface::Snapshot*                 snapshot,
                                  const Interface::Property*                 property,
                                  const bool                                 normalPermeability );

         /// Initialise the permeability-calculator by getting the ves and max-ves properties.
         bool initialise ();

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
