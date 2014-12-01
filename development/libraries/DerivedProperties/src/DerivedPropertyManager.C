#include "DerivedPropertyManager.h"

#include "AbstractProperty.h"

#include "Interface/Interface.h"
#include "Interface/Property.h"

#include "PrimarySurfaceProperty.h"
#include "PrimaryFormationSurfaceProperty.h"
#include "PrimaryFormationMapProperty.h"
#include "PrimaryFormationProperty.h"

#include "PrimarySurfacePropertyCalculator.h"
#include "PrimaryFormationMapPropertyCalculator.h"
#include "PrimaryFormationPropertyCalculator.h"
#include "PrimaryFormationSurfacePropertyCalculator.h"

DerivedProperties::DerivedPropertyManager::DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   loadSurfacePropertyCalculators ();
   loadFormationSurfacePropertyCalculators ();
   loadFormationMapPropertyCalculators ();
   loadFormationPropertyCalculators ();
}

const GeoPhysics::ProjectHandle* DerivedProperties::DerivedPropertyManager::getProjectHandle () const {
   return m_projectHandle;
}

const DataAccess::Interface::Property* DerivedProperties::DerivedPropertyManager::getProperty ( const std::string& name ) const {
   return m_projectHandle->findProperty ( name );
}

const DataAccess::Interface::Grid* DerivedProperties::DerivedPropertyManager::getMapGrid () const {
   return m_projectHandle->getActivityOutputGrid ();
}

void DerivedProperties::DerivedPropertyManager::loadSurfacePropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allSurfaceProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::SURFACE, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allSurfaceProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allSurfaceProperties)[ i ];

      PrimarySurfacePropertyCalculatorPtr propertyCalculator = PrimarySurfacePropertyCalculatorPtr ( new PrimarySurfacePropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      for ( ssIter = snapshots.begin (); ssIter != snapshots.end (); ++ssIter ) {
         addSurfacePropertyCalculator ( propertyCalculator, *ssIter );
      }

   } 

   delete allSurfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadFormationSurfacePropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationSurfaceProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATIONSURFACE, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allFormationSurfaceProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationSurfaceProperties)[ i ];

      PrimaryFormationSurfacePropertyCalculatorPtr propertyCalculator = PrimaryFormationSurfacePropertyCalculatorPtr ( new PrimaryFormationSurfacePropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      for ( ssIter = snapshots.begin (); ssIter != snapshots.end (); ++ssIter ) {
         addFormationSurfacePropertyCalculator ( propertyCalculator, *ssIter );
      }

   } 

   delete allFormationSurfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadFormationMapPropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationMapProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATION, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allFormationMapProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationMapProperties)[ i ];

      PrimaryFormationMapPropertyCalculatorPtr propertyCalculator = PrimaryFormationMapPropertyCalculatorPtr ( new PrimaryFormationMapPropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      for ( ssIter = snapshots.begin (); ssIter != snapshots.end (); ++ssIter ) {
         addFormationMapPropertyCalculator ( propertyCalculator, *ssIter );
      }

   } 

   delete allFormationMapProperties;
}

void DerivedProperties::DerivedPropertyManager::loadFormationPropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATION, 0, 0, 0, 0, DataAccess::Interface::VOLUME );

   for ( size_t i = 0; i < allFormationProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationProperties)[ i ];

      PrimaryFormationPropertyCalculatorPtr propertyCalculator ( new PrimaryFormationPropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      for ( ssIter = snapshots.begin (); ssIter != snapshots.end (); ++ssIter ) {
         addFormationPropertyCalculator ( propertyCalculator, *ssIter );
      }

   } 

   delete allFormationProperties;
}
