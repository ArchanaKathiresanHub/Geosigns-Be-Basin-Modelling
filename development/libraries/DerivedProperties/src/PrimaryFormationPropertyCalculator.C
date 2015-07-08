#include "PrimaryFormationPropertyCalculator.h"

#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "PrimaryFormationProperty.h"


DerivedProperties::PrimaryFormationPropertyCalculator::PrimaryFormationPropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                                                                            const DataModel::AbstractProperty* property ) :
   m_property ( property )
{

   DataAccess::Interface::PropertyValueList* formationProperties = projectHandle->getPropertyValues ( DataAccess::Interface::FORMATION, 0, 0, 0, 0, 0, DataAccess::Interface::VOLUME );
   addPropertyName ( property->getName ());

   for ( size_t i = 0; i < formationProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*formationProperties)[ i ];

      if ( propVal->getProperty () == m_property and propVal->getFormation () != 0 ) {
         m_snapshots.insert ( propVal->getSnapshot ());
         m_formationPropertyValues.push_back ( propVal );
      }

   }

   delete formationProperties;
}

DerivedProperties::PrimaryFormationPropertyCalculator::~PrimaryFormationPropertyCalculator () {
   m_formationPropertyValues.clear ();
}

void DerivedProperties::PrimaryFormationPropertyCalculator::calculate ( AbstractPropertyManager&            propManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation,
                                                                              FormationPropertyList&        derivedProperties ) const {

   (void) propManager;
   derivedProperties.clear ();

   for ( size_t i = 0; i < m_formationPropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_formationPropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getFormation () == formation and propVal->getSnapshot () == snapshot ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary formation property calculator.
         derivedProperties.push_back ( FormationPropertyPtr ( new PrimaryFormationProperty ( propVal )));
         break;
      }

   }

}

const DataModel::AbstractSnapshotSet& DerivedProperties::PrimaryFormationPropertyCalculator::getSnapshots () const {
   return m_snapshots;
} 
