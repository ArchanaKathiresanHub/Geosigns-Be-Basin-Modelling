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

// Derived formation-map property calculators
#include "AllochthonousLithologyFormationMapCalculator.h"
#include "FaultElementFormationMapCalculator.h"
#include "ThicknessFormationMapCalculator.h"

// Derived surface property calculators
#include "ReflectivitySurfaceCalculator.h"
#include "TwoWayTimeResidualSurfaceCalculator.h"

//utilities library
#include "FormattingException.h"

typedef formattingexception::GeneralException DerivedPropertyManagerException;
using namespace AbstractDerivedProperties;

DerivedProperties::DerivedPropertyManager::DerivedPropertyManager ( GeoPhysics::ProjectHandle& projectHandle,
                                                                    const bool                 loadAllProperties,
                                                                    const bool                 debug ) :
  m_projectHandle ( projectHandle )
{
  loadPrimaryFormationPropertyCalculators        ( loadAllProperties, debug );
  loadPrimaryFormationMapPropertyCalculators     ( loadAllProperties, debug );
  loadPrimarySurfacePropertyCalculators          ( loadAllProperties, debug );
  loadPrimaryFormationSurfacePropertyCalculators ( loadAllProperties, debug );
  loadPrimaryReservoirPropertyCalculators        ( debug );
  loadDerivedFormationPropertyCalculators        ( debug );
  loadDerivedFormationMapPropertyCalculators     ( debug );
  loadDerivedSurfacePropertyCalculators          ( debug );
}

const GeoPhysics::ProjectHandle& DerivedProperties::DerivedPropertyManager::getProjectHandle () const
{
  return m_projectHandle;
}

const DataModel::AbstractProperty* DerivedProperties::DerivedPropertyManager::getProperty ( const std::string& name ) const
{
  const DataAccess::Interface::Property* property = m_projectHandle.findProperty ( name );
  if (property == nullptr)
  {
    throw DerivedPropertyManagerException() << "Property '" << name << "' could not be found by the ProjectHandle.";
  }
  else
  {
    return property;
  }
}

const DataModel::AbstractGrid* DerivedProperties::DerivedPropertyManager::getMapGrid () const
{
  return m_projectHandle.getActivityOutputGrid ();
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedFormationPropertyCalculator ( const FormationPropertyCalculatorPtr& formationPropertyCalculator ) const
{
  if ( formationPropertyCalculator == nullptr )
  {
    return false;
  }

  for ( const std::string& propertyName : formationPropertyCalculator->getPropertyNames() )
  {
    // If any of the properties computed by the calculator are not currently computable then
    // the formation calculator needs to be added to the list of all formation calculators.
    if ( !formationPropertyIsComputable ( getProperty ( propertyName )))
    {
      return true;
    }
  }

  return false;
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr& formationMapPropertyCalculator ) const
{
  if ( formationMapPropertyCalculator == nullptr )
  {
    return false;
  }

  for ( const std::string& propertyName : formationMapPropertyCalculator->getPropertyNames () )
  {
    // If any of the properties computed by the calculator are not currently computable then
    // the formation-map calculator needs to be added to the list of formation-map calculators.
    if ( !formationMapPropertyIsComputable ( getProperty ( propertyName )))
    {
      return true;
    }
  }

  return false;
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedSurfacePropertyCalculator ( const SurfacePropertyCalculatorPtr& surfacePropertyCalculator ) const
{
  if ( surfacePropertyCalculator == nullptr )
  {
    return false;
  }

  for ( const std::string& propertyName : surfacePropertyCalculator->getPropertyNames() )
  {
    // If any of the properties computed by the calculator are not currently computable then
    // the surface calculator needs to be added to the list of surface calculators.
    if ( !surfacePropertyIsComputable ( getProperty ( propertyName )))
    {
      return true;
    }
  }

  return false;
}

bool DerivedProperties::DerivedPropertyManager::canAddDerivedFormationSurfacePropertyCalculator ( const AbstractDerivedProperties::FormationSurfacePropertyCalculatorPtr& formationSurfacePropertyCalculator ) const
{
  if ( formationSurfacePropertyCalculator == nullptr )
  {
    return false;
  }

  for ( const std::string& propertyName : formationSurfacePropertyCalculator->getPropertyNames() )
  {
    // If any of the properties computed by the calculator are not currently computable then
    // the formation surface calculator needs to be added to the list of formation surface calculators.
    if ( !formationSurfacePropertyIsComputable ( getProperty ( propertyName )))
    {
      return true;
    }
  }

  return false;
}

void  DerivedProperties::DerivedPropertyManager::loadDerivedFormationPropertyCalculator(FormationPropertyCalculatorPtr formationPropertyCalculator, const bool debug)
{
  if (canAddDerivedFormationPropertyCalculator(formationPropertyCalculator))
  {
    addFormationPropertyCalculator(formationPropertyCalculator, debug);
  }
}

void DerivedProperties::DerivedPropertyManager::loadDerivedFormationPropertyCalculators(const bool debug)
{
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
  if (canAddDerivedFormationMapPropertyCalculator(formationMapPropertyCalculator))
  {
    addFormationMapPropertyCalculator(formationMapPropertyCalculator);
  }
}

void DerivedProperties::DerivedPropertyManager::loadDerivedFormationMapPropertyCalculators(const bool debug)
{
  loadDerivedFormationMapPropertyCalculator(FormationMapPropertyCalculatorPtr(new AllochthonousLithologyFormationMapCalculator));

  loadDerivedFormationMapPropertyCalculator(FormationMapPropertyCalculatorPtr(new FaultElementFormationMapCalculator));

  loadDerivedFormationMapPropertyCalculator(FormationMapPropertyCalculatorPtr(new ThicknessFormationMapCalculator));
}


void  DerivedProperties::DerivedPropertyManager::loadDerivedSurfacePropertyCalculator(SurfacePropertyCalculatorPtr surfacePropertyCalculator)
{
  if (canAddDerivedSurfacePropertyCalculator(surfacePropertyCalculator))
  {
    addSurfacePropertyCalculator(surfacePropertyCalculator);
  }
}

void DerivedProperties::DerivedPropertyManager::loadDerivedSurfacePropertyCalculators(const bool /*debug*/)
{
  loadDerivedSurfacePropertyCalculator(SurfacePropertyCalculatorPtr(new ReflectivitySurfaceCalculator(m_projectHandle)));

  loadDerivedSurfacePropertyCalculator(SurfacePropertyCalculatorPtr(new TwoWayTimeResidualSurfaceCalculator(m_projectHandle)));
}

void  DerivedProperties::DerivedPropertyManager::loadDerivedFormationSurfacePropertyCalculator(FormationSurfacePropertyCalculatorPtr formationSurfacePropertyCalculator)
{
  if (canAddDerivedFormationSurfacePropertyCalculator(formationSurfacePropertyCalculator))
  {
    addFormationSurfacePropertyCalculator(formationSurfacePropertyCalculator);
  }
}

void DerivedProperties::DerivedPropertyManager::loadPrimarySurfacePropertyCalculators ( const bool loadAllProperties, const bool debug )
{
  DataAccess::Interface::PropertyPropertyValueListMap propertyMap =
        m_projectHandle.getPropertyPropertyValuesMap( DataAccess::Interface::SURFACE, DataAccess::Interface::MAP );
  for ( auto it : propertyMap )
  {
    const DataAccess::Interface::Property* property = it.first;

    if( loadAllProperties || ( property->isPrimary() && property->getPropertyAttribute() == DataModel::SURFACE_2D_PROPERTY ))
    {
      PrimarySurfacePropertyCalculatorPtr propertyCalculator = PrimarySurfacePropertyCalculatorPtr ( new PrimarySurfacePropertyCalculator ( property, it.second ));

      if ( debug )
      {
        std::cerr << " Adding surface primary property: " << property->getName () << std::endl;
      }

      addSurfacePropertyCalculator ( propertyCalculator );
    }
  }
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationSurfacePropertyCalculators ( const bool loadAllProperties, const bool debug )
{
  DataAccess::Interface::PropertyPropertyValueListMap propertyMap =
        m_projectHandle.getPropertyPropertyValuesMap( DataAccess::Interface::FORMATIONSURFACE, DataAccess::Interface::MAP );
  for ( auto it : propertyMap )
  {
    const DataAccess::Interface::Property* property = it.first;

    if( loadAllProperties || ( property->isPrimary() && ( property->getPropertyAttribute() == DataModel::SURFACE_2D_PROPERTY )))
    {
      PrimaryFormationSurfacePropertyCalculatorPtr propertyCalculator = PrimaryFormationSurfacePropertyCalculatorPtr ( new PrimaryFormationSurfacePropertyCalculator ( property, it.second ));

      if ( debug )
      {
        std::cerr << " Adding formation-surface primary property: " << property->getName () << std::endl;
      }

      addFormationSurfacePropertyCalculator ( propertyCalculator );
    }
  }
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationMapPropertyCalculators ( const bool loadAllProperties, const bool debug )
{
  DataAccess::Interface::PropertyPropertyValueListMap propertyMap =
        m_projectHandle.getPropertyPropertyValuesMap( DataAccess::Interface::FORMATION, DataAccess::Interface::MAP );
  for ( auto it : propertyMap )
  {
    const DataAccess::Interface::Property* property = it.first;

    if( loadAllProperties || ( property->isPrimary() && property->getPropertyAttribute() == DataModel::FORMATION_2D_PROPERTY )) {

      PrimaryFormationMapPropertyCalculatorPtr propertyCalculator = PrimaryFormationMapPropertyCalculatorPtr ( new PrimaryFormationMapPropertyCalculator ( property, it.second ));

      if ( debug )
      {
        std::cerr << " Adding formation-map primary property: " << property->getName () << std::endl;
      }

      addFormationMapPropertyCalculator ( propertyCalculator );
    }
  }
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryReservoirPropertyCalculators ( const bool debug )
{
  DataAccess::Interface::PropertyPropertyValueListMap propertyMap =
        m_projectHandle.getPropertyPropertyValuesMap( DataAccess::Interface::RESERVOIR, DataAccess::Interface::MAP );
  for ( auto it : propertyMap )
  {
    const DataAccess::Interface::Property* property = it.first;

    PrimaryReservoirPropertyCalculatorPtr propertyCalculator = PrimaryReservoirPropertyCalculatorPtr ( new PrimaryReservoirPropertyCalculator ( property, it.second ));

    if ( debug )
    {
      std::cerr << " Adding reservoir primary property: " << property->getName () << std::endl;
    }

    addReservoirPropertyCalculator ( propertyCalculator );
  }
}

void DerivedProperties::DerivedPropertyManager::loadPrimaryFormationPropertyCalculators ( const bool loadAllProperties, const bool debug )
{
  DataAccess::Interface::PropertyPropertyValueListMap propertyMap =
        m_projectHandle.getPropertyPropertyValuesMap( DataAccess::Interface::FORMATION, DataAccess::Interface::VOLUME );
  for ( auto it : propertyMap )
  {
    const DataAccess::Interface::Property* property = it.first;

    if( loadAllProperties || property->isPrimary() )
    {
      PrimaryFormationPropertyCalculatorPtr formationPropertyCalculator ( new PrimaryFormationPropertyCalculator ( property, it.second ));

      if ( debug )
      {
        std::cerr << " Adding formation primary property: " << property->getName () << std::endl;
      }

      addFormationPropertyCalculator ( formationPropertyCalculator, debug );
    }
  }
}


FormationPropertyList DerivedProperties::DerivedPropertyManager::getFormationProperties ( const DataModel::AbstractProperty* property,
                                                                                          const DataModel::AbstractSnapshot* snapshot,
                                                                                          const bool                         includeBasement )
{
  std::unique_ptr<DataAccess::Interface::FormationList> activeFormations( m_projectHandle.getFormations ( dynamic_cast <const DataAccess::Interface::Snapshot*>( snapshot ), includeBasement ) );

  FormationPropertyList results;
  results.reserve ( activeFormations->size ());

  for ( const DataAccess::Interface::Formation* activeFormation : *activeFormations )
  {
    FormationPropertyPtr formationProperty = getFormationProperty ( property, snapshot, activeFormation );

    if ( formationProperty != 0 )
    {
      results.push_back ( formationProperty );
    }
  }

  return results;
}

FormationMapPropertyList DerivedProperties::DerivedPropertyManager::getFormationMapProperties ( const DataModel::AbstractProperty* property,
                                                                                                const DataModel::AbstractSnapshot* snapshot,
                                                                                                const bool                         includeBasement )
{
  std::unique_ptr<DataAccess::Interface::FormationList> activeFormations( m_projectHandle.getFormations ( dynamic_cast <const DataAccess::Interface::Snapshot*>( snapshot ), includeBasement ) );

  FormationMapPropertyList results;
  results.reserve ( activeFormations->size ());

  for ( const DataAccess::Interface::Formation* activeFormation : *activeFormations )
  {
    FormationMapPropertyPtr formationProperty = getFormationMapProperty ( property, snapshot, activeFormation );

    if ( formationProperty != 0 )
    {
      results.push_back ( formationProperty );
    }
  }

  return results;
}



SurfacePropertyList DerivedProperties::DerivedPropertyManager::getSurfaceProperties ( const DataModel::AbstractProperty* property,
                                                                                      const DataModel::AbstractSnapshot* snapshot,
                                                                                      const bool                         includeBasement )
{
  std::unique_ptr<DataAccess::Interface::SurfaceList> activeSurfaces( m_projectHandle.getSurfaces ( dynamic_cast <const DataAccess::Interface::Snapshot*>( snapshot ), includeBasement ) );

  SurfacePropertyList results;
  results.reserve ( activeSurfaces->size ());

  for ( const DataAccess::Interface::Surface* activeSurface : *activeSurfaces )
  {
    SurfacePropertyPtr surfaceProperty = getSurfaceProperty ( property, snapshot, activeSurface );

    if ( surfaceProperty != 0 )
    {
      results.push_back ( surfaceProperty );
    }
  }

  return results;
}
