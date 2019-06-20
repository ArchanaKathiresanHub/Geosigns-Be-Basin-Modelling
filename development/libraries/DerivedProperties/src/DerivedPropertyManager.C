//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DerivedPropertyManager.h"

#include "AbstractProperty.h"

// DataAccess library.
#include "Interface.h"
#include "Property.h"
#include "Snapshot.h"
#include "Surface.h"

#include "PrimarySurfaceProperty.h"
#include "PrimaryFormationSurfaceProperty.h"
#include "PrimaryFormationMapProperty.h"
#include "PrimaryFormationProperty.h"

#include "PrimarySurfacePropertyCalculator.h"
#include "PrimaryFormationMapPropertyCalculator.h"
#include "PrimaryFormationPropertyCalculator.h"
#include "PrimaryFormationSurfacePropertyCalculator.h"
#include "PrimaryReservoirPropertyCalculator.h"

#include "FormationPropertyAtSurface.h"

#include "PropertyAttribute.h"

// Derived formation property calculators
#include "BrineDensityCalculator.h"
#include "BrineViscosityCalculator.h"
#include "BulkDensityFormationCalculator.h"
#include "FluidVelocityFormationCalculator.h"
#include "FracturePressureFormationCalculator.h"
#include "GammaRayFormationCalculator.h"
#include "HeatFlowFormationCalculator.h"
#include "HydrostaticPressureFormationCalculator.h"
#include "LithostaticPressureFormationCalculator.h"
#include "OverpressureFormationCalculator.h"
#include "PermeabilityFormationCalculator.h"
#include "PorosityFormationCalculator.h"
#include "PressureFormationCalculator.h"
#include "SonicFormationCalculator.h"
#include "ThermalConductivityFormationCalculator.h"
#include "ThermalDiffusivityFormationCalculator.h"
#include "TwoWayTimeFormationCalculator.h"
#include "VelocityFormationCalculator.h"

// Derived formation-map property calcualtors
#include "AllochthonousLithologyFormationMapCalculator.h"
#include "FaultElementFormationMapCalculator.h"
#include "ThicknessFormationMapCalculator.h"

// Derived surface property calcualtors
#include "ReflectivitySurfaceCalculator.h"
#include "TwoWayTimeResidualSurfaceCalculator.h"

//utilities library
#include "FormattingException.h"

typedef formattingexception::GeneralException DerivedPropertyManagerException;
using namespace AbstractDerivedProperties;

DerivedProperties::DerivedPropertyManager::DerivedPropertyManager ( GeoPhysics::ProjectHandle* projectHandle,
                                                                    const bool                 loadAllProperties,
                                                                    const bool                 debug ) : m_projectHandle ( projectHandle ) {

   m_loadAllProperties = loadAllProperties;
   loadPrimaryFormationPropertyCalculators        ( debug );
   loadPrimarySurfacePropertyCalculators          ( debug );
   loadPrimaryFormationSurfacePropertyCalculators ( debug );
   loadPrimaryFormationMapPropertyCalculators     ( debug );
   loadPrimaryReservoirPropertyCalculators        ( debug );
   loadDerivedFormationPropertyCalculators        ( debug );
   loadDerivedFormationMapPropertyCalculators     ( debug );
   loadDerivedSurfacePropertyCalculators          ( debug );
}

const GeoPhysics::ProjectHandle* DerivedProperties::DerivedPropertyManager::getProjectHandle () const {
   return m_projectHandle;
}

const DataAccess::Interface::Property* DerivedProperties::DerivedPropertyManager::getProperty ( const std::string& name ) const {
   const DataAccess::Interface::Property* property = m_projectHandle->findProperty ( name );
   if (property == 0) {
      throw DerivedPropertyManagerException() << "Property '" << name << "' could not be found by the ProjectHandle.";
   }
   else {
      return property;
   }
}

const DataAccess::Interface::Grid* DerivedProperties::DerivedPropertyManager::getMapGrid () const {
   return m_projectHandle->getActivityOutputGrid ();
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedFormationPropertyCalculator ( const FormationPropertyCalculatorPtr& formationPropertyCalculator ) const {

   if ( formationPropertyCalculator == 0 ) {
      return false;
   }

   const std::vector<std::string>& propertyNames = formationPropertyCalculator->getPropertyNames ();

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {

      // If any of the properties computed by the calculator are not currently computable then 
      // the formation calculator needs to be added to the list of all formation calculators.
      if ( not formationPropertyIsComputable ( getProperty ( propertyNames [ i ]))) {
         return true;
      }

   }

   return false;
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr& formationMapPropertyCalculator ) const {

   if ( formationMapPropertyCalculator == 0 ) {
      return false;
   }

   const std::vector<std::string>& propertyNames = formationMapPropertyCalculator->getPropertyNames ();

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {

      // If any of the properties computed by the calculator are not currently computable then 
      // the formation-map calculator needs to be added to the list of formation-map calculators.
      if ( not formationMapPropertyIsComputable ( getProperty ( propertyNames [ i ]))) {
         return true;
      }

   }

   return false;
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedSurfacePropertyCalculator ( const SurfacePropertyCalculatorPtr& surfacePropertyCalculator ) const {

   if ( surfacePropertyCalculator == 0 ) {
      return false;
   }

   const std::vector<std::string>& propertyNames = surfacePropertyCalculator->getPropertyNames ();

   for ( size_t i = 0; i < propertyNames.size (); ++i ) {

      // If any of the properties computed by the calculator are not currently computable then 
      // the surface calculator needs to be added to the list of surface calculators.
      if ( not surfacePropertyIsComputable ( getProperty ( propertyNames [ i ]))) {
         return true;
      }

   }

   return false;
}

void  DerivedProperties::DerivedPropertyManager::loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr formationPropertyCalculator, const bool debug)
{
   if (canAddDerivedFormationPropertyCalculator(formationPropertyCalculator)) {
      addFormationPropertyCalculator(formationPropertyCalculator, nullptr, debug);
   }
}

void DerivedProperties::DerivedPropertyManager::loadDerivedFormationPropertyCalculators(const bool debug) {

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new BrineDensityCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new BrineViscosityCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new BulkDensityFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new FracturePressureFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new GammaRayFormationCalculator), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new HydrostaticPressureFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new LithostaticPressureFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new OverpressureFormationCalculator), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new PermeabilityFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new PorosityFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new PressureFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new SonicFormationCalculator), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new ThermalConductivityFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new ThermalDiffusivityFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new TwoWayTimeFormationCalculator), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new VelocityFormationCalculator), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new HeatFlowFormationCalculator(m_projectHandle)), debug);

   loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr(new FluidVelocityFormationCalculator(m_projectHandle)), debug);
}

void  DerivedProperties::DerivedPropertyManager::loadDerivedFormationMapPropertyCalculator(FormationMapPropertyCalculatorPtr formationMapPropertyCalculator)
{
   if (canAddDerivedFormationMapPropertyCalculator(formationMapPropertyCalculator)) {
      addFormationMapPropertyCalculator(formationMapPropertyCalculator);
   }
}

void DerivedProperties::DerivedPropertyManager::loadDerivedFormationMapPropertyCalculators(const bool debug) {

   loadDerivedFormationMapPropertyCalculator(FormationMapPropertyCalculatorPtr(new AllochthonousLithologyFormationMapCalculator));

   loadDerivedFormationMapPropertyCalculator(FormationMapPropertyCalculatorPtr(new FaultElementFormationMapCalculator));

   loadDerivedFormationMapPropertyCalculator(FormationMapPropertyCalculatorPtr(new ThicknessFormationMapCalculator));

}


void  DerivedProperties::DerivedPropertyManager::loadDerivedSurfacePropertyCalculator(SurfacePropertyCalculatorPtr surfacePropertyCalculator)
{
   if (canAddDerivedSurfacePropertyCalculator(surfacePropertyCalculator)) {
      addSurfacePropertyCalculator(surfacePropertyCalculator);
   }
}

void DerivedProperties::DerivedPropertyManager::loadDerivedSurfacePropertyCalculators(const bool debug) {

   loadDerivedSurfacePropertyCalculator(SurfacePropertyCalculatorPtr(new ReflectivitySurfaceCalculator(m_projectHandle)));

   loadDerivedSurfacePropertyCalculator(SurfacePropertyCalculatorPtr(new TwoWayTimeResidualSurfaceCalculator(m_projectHandle)));

}

void DerivedProperties::DerivedPropertyManager::loadPrimarySurfacePropertyCalculators ( const bool debug ) {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allSurfaceProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::SURFACE, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allSurfaceProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allSurfaceProperties)[ i ];

      if( m_loadAllProperties or ( property->isPrimary() and property->getPropertyAttribute() == DataModel::SURFACE_2D_PROPERTY )) {
         PrimarySurfacePropertyCalculatorPtr propertyCalculator = PrimarySurfacePropertyCalculatorPtr ( new PrimarySurfacePropertyCalculator ( m_projectHandle, property ));
         const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
         DataModel::AbstractSnapshotSet::const_iterator ssIter;
         
         if ( debug ) {
            std::cerr << " Adding surface primary property: " << property->getName () << std::endl;
         }

         addSurfacePropertyCalculator ( propertyCalculator );
      } 
   }

   delete allSurfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationSurfacePropertyCalculators ( const bool debug ) {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationSurfaceProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATIONSURFACE, 0, 0, 0, 0, DataAccess::Interface::MAP );


   for ( size_t i = 0; i < allFormationSurfaceProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationSurfaceProperties)[ i ];

      if( m_loadAllProperties or
          ( property->isPrimary() and ( property->getPropertyAttribute() == DataModel::SURFACE_2D_PROPERTY ))) {

         PrimaryFormationSurfacePropertyCalculatorPtr propertyCalculator = PrimaryFormationSurfacePropertyCalculatorPtr ( new PrimaryFormationSurfacePropertyCalculator ( m_projectHandle, property ));
         const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
         DataModel::AbstractSnapshotSet::const_iterator ssIter;
         
         if ( debug ) {
            std::cerr << " Adding formation-surface primary property: " << property->getName () << std::endl;
         }
         
         addFormationSurfacePropertyCalculator ( propertyCalculator );
      } 
   }

   delete allFormationSurfaceProperties;
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationMapPropertyCalculators ( const bool debug ) {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationMapProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATION, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allFormationMapProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationMapProperties)[ i ];

      if( m_loadAllProperties or ( property->isPrimary() and property->getPropertyAttribute() == DataModel::FORMATION_2D_PROPERTY )) {
 
         PrimaryFormationMapPropertyCalculatorPtr propertyCalculator = PrimaryFormationMapPropertyCalculatorPtr ( new PrimaryFormationMapPropertyCalculator ( m_projectHandle, property ));
         const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
         DataModel::AbstractSnapshotSet::const_iterator ssIter;
         
         if ( debug ) {
            std::cerr << " Adding formation-map primary property: " << property->getName () << std::endl;
         }
         
         addFormationMapPropertyCalculator ( propertyCalculator );
      } 
   }

   delete allFormationMapProperties;
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryReservoirPropertyCalculators ( const bool debug ) {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allReservoirProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::RESERVOIR, 0, 0, 0, 0, DataAccess::Interface::MAP );

   for ( size_t i = 0; i < allReservoirProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allReservoirProperties)[ i ];

      PrimaryReservoirPropertyCalculatorPtr propertyCalculator = PrimaryReservoirPropertyCalculatorPtr ( new PrimaryReservoirPropertyCalculator ( m_projectHandle, property ));
      const DataModel::AbstractSnapshotSet& snapshots = propertyCalculator->getSnapshots ();
      DataModel::AbstractSnapshotSet::const_iterator ssIter;

      if ( debug ) {
         std::cerr << " Adding reservoir primary property: " << property->getName () << std::endl;
      }

      addReservoirPropertyCalculator ( propertyCalculator );
   } 

   delete allReservoirProperties;
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationPropertyCalculators ( const bool debug ) {

   // Get a list of properties that have been saved.
   DataAccess::Interface::PropertyList* allFormationProperties = m_projectHandle->getProperties ( false, DataAccess::Interface::FORMATION, 0, 0, 0, 0, DataAccess::Interface::VOLUME );

   for ( size_t i = 0; i < allFormationProperties->size (); ++i ) {
      const DataAccess::Interface::Property* property = (*allFormationProperties)[ i ];

      if( m_loadAllProperties or property->isPrimary() ) {
         PrimaryFormationPropertyCalculatorPtr formationPropertyCalculator ( new PrimaryFormationPropertyCalculator ( m_projectHandle, property ));
         const DataModel::AbstractSnapshotSet& snapshots = formationPropertyCalculator->getSnapshots ();
         DataModel::AbstractSnapshotSet::const_iterator ssIter;
         
         if ( debug ) {
            std::cerr << " Adding formation primary property: " << property->getName () << std::endl;
         }
         
         addFormationPropertyCalculator ( formationPropertyCalculator, 0, debug );
      } 
   }

   delete allFormationProperties;
}


FormationPropertyList DerivedProperties::DerivedPropertyManager::getFormationProperties ( const DataModel::AbstractProperty* property,
                                                                                                             const DataModel::AbstractSnapshot* snapshot,
                                                                                                             const bool                         includeBasement ) {

   DataAccess::Interface::FormationList* activeFormations = m_projectHandle->getFormations ( dynamic_cast <const DataAccess::Interface::Snapshot*>( snapshot ), includeBasement );
   FormationPropertyList results;

   results.reserve ( activeFormations->size ());

   for ( size_t i = 0; i < activeFormations->size (); ++i ) {
      FormationPropertyPtr formationProperty = getFormationProperty ( property, snapshot, activeFormations->at ( i ));

      if ( formationProperty != 0 ) {
         results.push_back ( formationProperty );
      }

   }

   delete activeFormations;
   return results;
}

FormationMapPropertyList DerivedProperties::DerivedPropertyManager::getFormationMapProperties ( const DataModel::AbstractProperty* property,
                                                                                                                   const DataModel::AbstractSnapshot* snapshot,
                                                                                                                   const bool                         includeBasement ) {

   DataAccess::Interface::FormationList* activeFormations = m_projectHandle->getFormations ( dynamic_cast <const DataAccess::Interface::Snapshot*>( snapshot ), includeBasement );
   FormationMapPropertyList results;

   results.reserve ( activeFormations->size ());

   for ( size_t i = 0; i < activeFormations->size (); ++i ) {
      FormationMapPropertyPtr formationProperty = getFormationMapProperty ( property, snapshot, activeFormations->at ( i ));

      if ( formationProperty != 0 ) {
         results.push_back ( formationProperty );
      }

   }

   delete activeFormations;
   return results;
}



SurfacePropertyList DerivedProperties::DerivedPropertyManager::getSurfaceProperties ( const DataModel::AbstractProperty* property,
                                                                                                         const DataModel::AbstractSnapshot* snapshot,
                                                                                                         const bool                         includeBasement ) {

   DataAccess::Interface::SurfaceList* activeSurfaces = m_projectHandle->getSurfaces ( dynamic_cast <const DataAccess::Interface::Snapshot*>( snapshot ), includeBasement );
   SurfacePropertyList results;

   results.reserve ( activeSurfaces->size ());

   for ( size_t i = 0; i < activeSurfaces->size (); ++i ) {
      SurfacePropertyPtr surfaceProperty = getSurfaceProperty ( property, snapshot, activeSurfaces->at ( i ));

      if ( surfaceProperty != 0 ) {
         results.push_back ( surfaceProperty );
      }

   }

   delete activeSurfaces;
   return results;
}
