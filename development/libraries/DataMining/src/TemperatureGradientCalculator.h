#ifndef _MINING__TEMPERATURE_GRADIENT_CALCULATOR__H_
#define _MINING__TEMPERATURE_GRADIENT_CALCULATOR__H_

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

      class TemperatureGradientCalculator : public DomainFormationProperty {

      public :

         TemperatureGradientCalculator ( const DomainPropertyCollection*            collection,
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

         const DomainFormationProperty* m_depth;
         const DomainFormationProperty* m_temperature;
         bool                           m_initialised;

      };

      /// Functor class for allocating DomainFormationProperty objects.
      class TemperatureGradientCalculatorAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };

   }

}


#endif // _MINING__TEMPERATURE_GRADIENT_CALCULATOR__H_
