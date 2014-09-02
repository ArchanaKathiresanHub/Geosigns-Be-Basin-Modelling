#include "DerivedPropertyManager.h"

#include "AbstractProperty.h"

#include "Interface/Interface.h"
#include "Interface/Property.h"

#include "PrimarySurfaceProperty.h"

DerivedProperties::DerivedPropertyManager::DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   loadSurfaceProperties ();
}

const DataModel::AbstractProperty* DerivedProperties::DerivedPropertyManager::getProperty ( const std::string& name ) const {
   return m_projectHandle->findProperty ( name );
}

void DerivedProperties::DerivedPropertyManager::loadSurfaceProperties () {

   DataAccess::Interface::PropertyValueList* surfaceProperties = m_projectHandle->getPropertyValues ( DataAccess::SURFACE, 0, 0, 0, 0, 0, DataAccess::MAP );

   for ( size_t i = 0; i < surfaceProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*surfaceProperties)[ i ];

      // Only add those property-values that are strictly for the surface only, i.e. not surface-formation.
      if ( propVal->getSurface () != 0 and propVal->getFormation () == 0 ) {
         addSurfaceProperty ( SurfacePropertyPtr ( new PrimarySurfaceProperty ( propVal )));
      }

   }

   delete surfaceProperties;
}
