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

#include "FormationPropertyAtSurface.h"

#include "PropertyAttribute.h"

// Derived formation property calculators
#include "FracturePressureFormationCalculator.h"
#include "HydrostaticPressureFormationCalculator.h"
#include "LithostaticPressureFormationCalculator.h"
#include "OverpressureFormationCalculator.h"
#include "PermeabilityFormationCalculator.h"
#include "PorosityFormationCalculator.h"
#include "ThermalConductivityFormationCalculator.h"
#include "ThermalDiffusivityFormationCalculator.h"
#include "VelocityFormationCalculator.h"

// Derived formation-map property calcualtors
#include "AllochthonousLithologyFormationMapCalculator.h"
#include "ErosionFactorFormationMapCalculator.h"
#include "FaultElementFormationMapCalculator.h"
#include "ThicknessFormationMapCalculator.h"

DerivedProperties::DerivedPropertyManager::DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   loadPrimaryFormationPropertyCalculators ();
   loadPrimarySurfacePropertyCalculators ();
   loadPrimaryFormationSurfacePropertyCalculators ();
   loadPrimaryFormationMapPropertyCalculators ();

   loadDerivedFormationPropertyCalculator ();
   loadDerivedFormationMapPropertyCalculator ();
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

bool DerivedProperties::DerivedPropertyManager::canAddDerivedFormationPropertyCalculator ( const FormationPropertyCalculatorPtr& formationPropertyCalculator) const {

   const std::vector<std::string>& propertyNames = formationPropertyCalculator->getPropertyNames ();

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {

      // If any of the properties computed by the calculator are not currently computable then 
      // the calculator need to be added to the list of calculators.
      if ( not formationPropertyIsComputable ( getProperty ( propertyNames [ i ]))) {
         return true;
      }

   }

   return false;
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr& formationMapPropertyCalculator) const {

   const std::vector<std::string>& propertyNames = formationMapPropertyCalculator->getPropertyNames ();

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {

      // If any of the properties computed by the calculator are not currently computable then 
      // the calculator need to be added to the list of calculators.
      if ( not formationMapPropertyIsComputable ( getProperty ( propertyNames [ i ]))) {
         return true;
      }

   }

   return false;
}

void DerivedProperties::DerivedPropertyManager::loadDerivedFormationPropertyCalculator () {

   FormationPropertyCalculatorPtr formationPropertyCalculator;

   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new FracturePressureFormationCalculator ( m_projectHandle ));

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new HydrostaticPressureFormationCalculator ( m_projectHandle ));

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new LithostaticPressureFormationCalculator ( m_projectHandle ));

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new OverpressureFormationCalculator );

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new PermeabilityFormationCalculator ( m_projectHandle ));

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new PorosityFormationCalculator ( m_projectHandle ));

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new ThermalConductivityFormationCalculator ( m_projectHandle ));

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new ThermalDiffusivityFormationCalculator ( m_projectHandle ));

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }
   
   formationPropertyCalculator = FormationPropertyCalculatorPtr ( new VelocityFormationCalculator );

   if ( canAddDerivedFormationPropertyCalculator ( formationPropertyCalculator )) {
      addFormationPropertyCalculator ( formationPropertyCalculator );
   }

}

void DerivedProperties::DerivedPropertyManager::loadDerivedFormationMapPropertyCalculator () {

   FormationMapPropertyCalculatorPtr formationMapPropertyCalculator;

   formationMapPropertyCalculator = FormationMapPropertyCalculatorPtr ( new AllochthonousLithologyFormationMapCalculator );

   if ( canAddDerivedFormationMapPropertyCalculator ( formationMapPropertyCalculator )) {
      addFormationMapPropertyCalculator ( formationMapPropertyCalculator );
   }

   formationMapPropertyCalculator = FormationMapPropertyCalculatorPtr ( new ErosionFactorFormationMapCalculator );

   if ( canAddDerivedFormationMapPropertyCalculator ( formationMapPropertyCalculator )) {
      addFormationMapPropertyCalculator ( formationMapPropertyCalculator );
   }

   formationMapPropertyCalculator = FormationMapPropertyCalculatorPtr ( new FaultElementFormationMapCalculator );

   if ( canAddDerivedFormationMapPropertyCalculator ( formationMapPropertyCalculator )) {
      addFormationMapPropertyCalculator ( formationMapPropertyCalculator );
   }

   formationMapPropertyCalculator = FormationMapPropertyCalculatorPtr ( new ThicknessFormationMapCalculator );

   if ( canAddDerivedFormationMapPropertyCalculator ( formationMapPropertyCalculator )) {
      addFormationMapPropertyCalculator ( formationMapPropertyCalculator );
   }

}

void DerivedProperties::DerivedPropertyManager::loadPrimarySurfacePropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allSurfaceProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::SURFACE, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allSurfaceProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allSurfaceProperties)[ i ];

      PrimarySurfacePropertyCalculatorPtr propertyCalculator = PrimarySurfacePropertyCalculatorPtr ( new PrimarySurfacePropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      addSurfacePropertyCalculator ( propertyCalculator );
   } 

   delete allSurfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationSurfacePropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationSurfaceProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATIONSURFACE, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allFormationSurfaceProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationSurfaceProperties)[ i ];

      PrimaryFormationSurfacePropertyCalculatorPtr propertyCalculator = PrimaryFormationSurfacePropertyCalculatorPtr ( new PrimaryFormationSurfacePropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      addFormationSurfacePropertyCalculator ( propertyCalculator );
   } 

   delete allFormationSurfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationMapPropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationMapProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATION, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allFormationMapProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationMapProperties)[ i ];

      PrimaryFormationMapPropertyCalculatorPtr propertyCalculator = PrimaryFormationMapPropertyCalculatorPtr ( new PrimaryFormationMapPropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      addFormationMapPropertyCalculator ( propertyCalculator );
   } 

   delete allFormationMapProperties;
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationPropertyCalculators () {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATION, 0, 0, 0, 0, DataAccess::Interface::VOLUME );

   for ( size_t i = 0; i < allFormationProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationProperties)[ i ];

      PrimaryFormationPropertyCalculatorPtr formationPropertyCalculator ( new PrimaryFormationPropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = formationPropertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      addFormationPropertyCalculator ( formationPropertyCalculator );
   } 

   delete allFormationProperties;
}
