#ifndef _MINING__DOMAIN_POROSITY_CALCULATOR__H_
#define _MINING__DOMAIN_POROSITY_CALCULATOR__H_

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

      class PorosityCalculator : public DomainFormationProperty {

      public :

         PorosityCalculator ( const DomainPropertyCollection*            collection,
                              DerivedProperties::DerivedPropertyManager& propertyManager,
                              const Interface::Snapshot*                 snapshot,
                              const Interface::Property*                 property );

         /// Initialise the porosity-calculator by getting the ves and max-ves properties.
         bool initialise ();

         /// \brief Compute the porosity.
         ///
         /// By first interpolating the ves and max-ves then evaluating the porosity function
         /// in the lithology defined at the element position within the formation.
         double compute ( const ElementPosition& position ) const;

      private :

         const DomainProperty* m_ves;
         const DomainProperty* m_maxVes;
         const DomainProperty* m_chemicalCompaction;
         bool                  m_chemicalCompactionRequired;
         bool                  m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class PorosityCalculatorAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };

   }

}

#endif // _MINING__DOMAIN_POROSITY_CALCULATOR__H_
