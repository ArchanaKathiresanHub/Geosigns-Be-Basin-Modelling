#include "PrimaryFormationSurfacePropertyCalculator.h"

#include "Interface/Formation.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

#include "PrimaryFormationSurfaceProperty.h"

DerivedProperties::PrimaryFormationSurfacePropertyCalculator::PrimaryFormationSurfacePropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                                                                                          const DataModel::AbstractProperty* property ) :
   FormationSurfacePropertyCalculator ( projectHandle ),
   m_property ( property )
{

   DataAccess::Interface::PropertyValueList* formationProperties = projectHandle->getPropertyValues ( DataAccess::Interface::FORMATION, 0, 0, 0, 0, 0, DataAccess::Interface::MAP );
   m_propertyNames.push_back ( property->getName ());

   for ( size_t i = 0; i < formationProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*formationProperties)[ i ];

      if ( propVal->getProperty () == m_property and propVal->getFormation () != 0 and propVal->getSurface () != 0 ) {
         m_snapshots.insert ( propVal->getSnapshot ());
         m_formationSurfacePropertyValues.push_back ( propVal );
      }

   }

   delete formationProperties;
}

DerivedProperties::PrimaryFormationSurfacePropertyCalculator::~PrimaryFormationSurfacePropertyCalculator () {
   m_formationSurfacePropertyValues.clear ();
}

void DerivedProperties::PrimaryFormationSurfacePropertyCalculator::calculate ( AbstractPropertyManager&      propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation,
                                                                               const DataModel::AbstractSurface*   surface,
                                                                               FormationSurfacePropertyList& derivedProperties ) const {

   (void) propManager;
   derivedProperties.clear ();

   for ( size_t i = 0; i < m_formationSurfacePropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_formationSurfacePropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getFormation () == formation and propVal->getSurface () == surface and propVal->getSnapshot () == snapshot ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary formation property calculator.
         derivedProperties.push_back ( FormationSurfacePropertyPtr ( new PrimaryFormationSurfaceProperty ( propVal )));
         break;
      }

   }

}

const std::vector<std::string>& DerivedProperties::PrimaryFormationSurfacePropertyCalculator::getPropertyNames () const {
   return m_propertyNames;
} 

const DataModel::AbstractSnapshotSet& DerivedProperties::PrimaryFormationSurfacePropertyCalculator::getSnapshots () const {
   return m_snapshots;
} 
