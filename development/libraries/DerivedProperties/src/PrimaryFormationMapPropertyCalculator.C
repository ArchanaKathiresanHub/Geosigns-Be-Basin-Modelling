#include "PrimaryFormationMapPropertyCalculator.h"

#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "PrimaryFormationMapProperty.h"

DerivedProperties::PrimaryFormationMapPropertyCalculator::PrimaryFormationMapPropertyCalculator ( const DataAccess::Interface::ProjectHandle*   projectHandle,
                                                                                                  const DataModel::AbstractProperty* property ) :
   m_property ( property )
{

   DataAccess::Interface::PropertyValueList* formationProperties = projectHandle->getPropertyValues ( DataAccess::Interface::FORMATION, 0, 0, 0, 0, 0, DataAccess::Interface::MAP );
   m_propertyNames.push_back ( property->getName ());

   for ( size_t i = 0; i < formationProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*formationProperties)[ i ];

      if ( propVal->getProperty () == m_property and propVal->getSurface () == 0 and propVal->getFormation () != 0 ) {
         m_formationPropertyValues.push_back ( propVal );
      }

   }

   delete formationProperties;
}

DerivedProperties::PrimaryFormationMapPropertyCalculator::~PrimaryFormationMapPropertyCalculator () {
   m_formationPropertyValues.clear ();
}

void DerivedProperties::PrimaryFormationMapPropertyCalculator::calculate ( AbstractPropertyManager&            propManager,
                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                           const DataModel::AbstractFormation* formation,
                                                                           FormationMapPropertyList&           derivedProperties ) const {

   derivedProperties.clear ();

   for ( size_t i = 0; i < m_formationPropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_formationPropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getFormation () == formation and propVal->getSnapshot () == snapshot ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary formation property calculator.
         derivedProperties.push_back ( FormationMapPropertyPtr ( new PrimaryFormationMapProperty ( propVal )));
         break;
      }

   }

}

const std::vector<std::string>& DerivedProperties::PrimaryFormationMapPropertyCalculator::getPropertyNames () const {
   return m_propertyNames;
} 
