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
   addPropertyName ( property->getName ());

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
   (void) propManager;

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

bool DerivedProperties::PrimarySurfacePropertyCalculator::isComputable ( const AbstractPropertyManager&     propManager,
                                                                         const DataModel::AbstractSnapshot* snapshot,
                                                                         const DataModel::AbstractSurface*  surface ) const {

   (void) propManager;

   for ( size_t i = 0; i < m_surfacePropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_surfacePropertyValues [ i ];

      if ( propVal->getProperty () == m_property and ( surface == 0 or propVal->getSurface () == surface ) and ( snapshot == 0 or propVal->getSnapshot () == snapshot )) {
         return true;
      }

   }

   return false;
}

const DataModel::AbstractSnapshotSet& DerivedProperties::PrimarySurfacePropertyCalculator::getSnapshots () const {
   return m_snapshots;
} 
