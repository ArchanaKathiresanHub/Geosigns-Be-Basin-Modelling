#include "DerivedPropertyManager.h"

#include "AbstractProperty.h"

#include "Interface/Interface.h"
#include "Interface/Property.h"

#include "PrimarySurfaceProperty.h"
#include "PrimaryFormationSurfaceProperty.h"
#include "PrimaryFormationProperty.h"

DerivedProperties::DerivedPropertyManager::DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   loadSurfaceProperties ();
   loadFormationSurfaceProperties ();
   loadFormationProperties ();
}

const DataModel::AbstractProperty* DerivedProperties::DerivedPropertyManager::getProperty ( const std::string& name ) const {
   return m_projectHandle->findProperty ( name );
}

void DerivedProperties::DerivedPropertyManager::loadSurfaceProperties () {

   DataAccess::Interface::PropertyValueList* surfaceProperties = m_projectHandle->getPropertyValues ( DataAccess::SURFACE, 0, 0, 0, 0, 0, DataAccess::MAP );

   for ( size_t i = 0; i < surfaceProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*surfaceProperties)[ i ];

      // Only add those property-values that are strictly for the surface only, i.e. not surface-formation and not formation only.
      if ( propVal->getSurface () != 0 and propVal->getFormation () == 0 ) {
         addSurfaceProperty ( SurfacePropertyPtr ( new PrimarySurfaceProperty ( propVal )));
      }

   }

   delete surfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadFormationSurfaceProperties () {

   DataAccess::Interface::PropertyValueList* formationSurfaceProperties = m_projectHandle->getPropertyValues ( DataAccess::SURFACE | DataAccess::FORMATION, 0, 0, 0, 0, 0, DataAccess::MAP );

   for ( size_t i = 0; i < formationSurfaceProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*formationSurfaceProperties)[ i ];

      // Only add those property-values that are strictly for the surface-formation, i.e. not surface only and not formation only.
      if ( propVal->getSurface () != 0 and propVal->getFormation () != 0 ) {
         addFormationSurfaceProperty ( FormationSurfacePropertyPtr ( new PrimaryFormationSurfaceProperty ( propVal )));
      }

   }

   delete formationSurfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadFormationProperties () {

   DataAccess::Interface::PropertyValueList* formationProperties = m_projectHandle->getPropertyValues ( DataAccess::FORMATION, 0, 0, 0, 0, 0, DataAccess::VOLUME );

   for ( size_t i = 0; i < formationProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*formationProperties)[ i ];

      if ( propVal->getFormation () != 0 ) {
         addFormationProperty ( FormationPropertyPtr ( new PrimaryFormationProperty ( propVal )));
      }

   }

   delete formationProperties;
}
