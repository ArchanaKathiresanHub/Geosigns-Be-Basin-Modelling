#include "PrimarySurfacePropertyCalculator.h"

#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "PrimarySurfaceProperty.h"

DerivedProperties::PrimarySurfacePropertyCalculator::PrimarySurfacePropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                                                                        const DataModel::AbstractProperty* property ) :
   m_property ( property )
{

   DataAccess::Interface::PropertyValueList* surfaceProperties = projectHandle->getPropertyValues ( DataAccess::Interface::SURFACE, 0, 0, 0, 0, 0, DataAccess::Interface::MAP );
   m_propertyNames.push_back ( property->getName ());

   for ( size_t i = 0; i < surfaceProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*surfaceProperties)[ i ];

      if ( propVal->getProperty () == m_property and propVal->getSurface () != 0 and propVal->getFormation () == 0 ) {
         m_snapshots.insert ( propVal->getSnapshot ());
         m_surfacePropertyValues.push_back ( propVal );
      }

   }

   delete surfaceProperties;
}

DerivedProperties::PrimarySurfacePropertyCalculator::~PrimarySurfacePropertyCalculator () {
   m_surfacePropertyValues.clear ();
}

void DerivedProperties::PrimarySurfacePropertyCalculator::calculate ( AbstractPropertyManager&           propManager,
                                                                      const DataModel::AbstractSnapshot* snapshot,
                                                                      const DataModel::AbstractSurface*  surface,
                                                                            SurfacePropertyList&         derivedProperties ) const {

   derivedProperties.clear ();

   for ( size_t i = 0; i < m_surfacePropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_surfacePropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getSurface () == surface and propVal->getSnapshot () == snapshot ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary surface property calculator.
         derivedProperties.push_back ( SurfacePropertyPtr ( new PrimarySurfaceProperty ( propVal )));
         break;
      }

   }

}

const std::vector<std::string>& DerivedProperties::PrimarySurfacePropertyCalculator::getPropertyNames () const {
   return m_propertyNames;
} 

const DataModel::AbstractSnapshotSet& DerivedProperties::PrimarySurfacePropertyCalculator::getSnapshots () const {
   return m_snapshots;
} 
