#include "derivedPropertyDriller.h"
#include "datadriller.h"

#include "cauldronschemafuncs.h"
#include "errorhandling.h"

// DataAccess library
#include "Formation.h"
#include "ProjectHandle.h"
#include "Surface.h"
#include "Property.h"

// Derived Properties library
#include "FormationPropertyCalculator.h"

#include "BrineDensityCalculator.h"
#include "BrineViscosityCalculator.h"
#include "BulkDensityFormationCalculator.h"
#include "GammaRayFormationCalculator.h"
#include "LithostaticPressureFormationCalculator.h"
#include "PermeabilityFormationCalculator.h"
#include "PorosityFormationCalculator.h"
#include "SonicFormationCalculator.h"
#include "ThermalConductivityFormationCalculator.h"
#include "ThermalDiffusivityFormationCalculator.h"
#include "VelocityFormationCalculator.h"

// Utility library
#include "ConstantsNumerical.h"
#include <cmath>

#include "GeoPhysicsFormation.h"

using namespace DataAccess::Interface;
using namespace Utilities::Numerical;
using namespace DataExtraction;
using namespace database;

DerivedPropertyDriller::DerivedPropertyDriller( const DataDriller* dataDriller ) :
  m_record{nullptr},
  m_snapshot{nullptr},
  m_obtainedProperties{},
  m_dataDriller{dataDriller}
{
}

void DerivedPropertyDriller::setRecord(Record *record)
{
  m_record = record;
}

void DerivedPropertyDriller::setSnapshot(const Snapshot *snapshot)
{
  m_snapshot = snapshot;
}

bool DerivedPropertyDriller::run( double& value )
{
  m_obtainedProperties.clear();
  const std::vector<std::string> dependentProperties = defineDependentProperties( getPropertyName( m_record) );
  obtainDependentProperties( dependentProperties );
  value = calculateProperty( getPropertyName( m_record ) );
  return std::fabs(value - DefaultUndefinedScalarValue) > 1e-4;
}

double DerivedPropertyDriller::run(const std::string& alternativePropertyName)
{
  const std::vector<std::string> dependentProperties = defineDependentProperties(alternativePropertyName);
  obtainDependentProperties(dependentProperties);
  return calculateProperty(alternativePropertyName);
}

AbstractDerivedProperties::FormationPropertyCalculator* DerivedPropertyDriller::initializeCalculator(const std::string& propertyName) const
{
  const GeoPhysics::ProjectHandle& geophysicsHandle = dynamic_cast<GeoPhysics::ProjectHandle&>(m_dataDriller->getProjectHandle());

  if (propertyName == "BrineViscosity") {
    return new DerivedProperties::BrineViscosityCalculator ( geophysicsHandle );
  }
  if (propertyName == "BrineDensity") {
    return new DerivedProperties::BrineDensityCalculator( geophysicsHandle );
  }
  if (propertyName == "Porosity") {
    return new DerivedProperties::PorosityFormationCalculator( geophysicsHandle );
  }
  if (propertyName == "LithoStaticPressure") {
    return new DerivedProperties::LithostaticPressureFormationCalculator( geophysicsHandle );
  }
  if (propertyName == "Velocity") {
    return new DerivedProperties::VelocityFormationCalculator();
  }
  if (propertyName == "Permeability") {
    return new DerivedProperties::PermeabilityFormationCalculator( geophysicsHandle );
  }
  if (propertyName == "DiffusivityVec2") {
    return new DerivedProperties::ThermalDiffusivityFormationCalculator( geophysicsHandle );
  }
  if (propertyName == "BulkDensity") {
    return new DerivedProperties::BulkDensityFormationCalculator( geophysicsHandle );
  }
  if (propertyName == "SonicSlowness") {
    return new DerivedProperties::SonicFormationCalculator();
  }
  if (propertyName == "ThCondVec2") {
    return new DerivedProperties::ThermalConductivityFormationCalculator( geophysicsHandle );
  }
  if (propertyName == "GammaRay") {
    return new DerivedProperties::GammaRayFormationCalculator();
  }

  return nullptr;
}

std::vector<std::string> DerivedPropertyDriller::defineDependentProperties(const std::string& propertyName) const
{
  std::unique_ptr<AbstractDerivedProperties::FormationPropertyCalculator> calculator(initializeCalculator(propertyName));
  if (calculator)
  {
    return calculator->getDependentPropertyNames();
  }

  return {};
}

double DerivedPropertyDriller::obtainValue(const std::string& propertyName)
{
  const Property* property = m_dataDriller->getProjectHandle().findProperty(propertyName);
  if ( m_snapshot != nullptr && property != nullptr )
  {
    double value = DefaultUndefinedScalarValue;
    if ( property->isPrimary() &&
         m_dataDriller->readPropertyFromHDF( getSurfaceName( m_record ), getFormationName( m_record ),
                                             getXCoord( m_record ), getYCoord( m_record ), getZCoord( m_record ),
                                             m_snapshot, property, value ) )
    {
      return value;
    }
    else
    {
      return run( propertyName );
    }
  }

  return DefaultUndefinedScalarValue;
}

void DerivedPropertyDriller::obtainDependentProperties( const std::vector<std::string>& dependentProperties )
{
  if ( dependentProperties.empty() )
  {
    return;
  }
  for ( const std::string& dependentProperty: dependentProperties)
  {
    if ( m_obtainedProperties.find(dependentProperty) == m_obtainedProperties.end() )
    {
      m_obtainedProperties.insert({dependentProperty, obtainValue(dependentProperty)});
    }
  }
}

const GeoPhysics::CompoundLithology* DerivedPropertyDriller::getLithology(const unsigned int i, const unsigned int j, const GeoPhysics::GeoPhysicsFormation* formation ) const
{
  if ( !formation )
  {
    return nullptr;
  }
  const DataAccess::Interface::GridMap* lithoMap1 = formation->getLithoType1PercentageMap ();
  const DataAccess::Interface::GridMap* lithoMap2 = formation->getLithoType2PercentageMap ();
  const DataAccess::Interface::GridMap* lithoMap3 = formation->getLithoType3PercentageMap ();

  const std::string lithoName1 = ( formation->getLithoType1 () ) ? formation->getLithoType1 ()->getName () : "";
  const std::string lithoName2 = ( formation->getLithoType2 () ) ? formation->getLithoType2 ()->getName () : "";
  const std::string lithoName3 = ( formation->getLithoType3 () ) ? formation->getLithoType3 ()->getName () : "";

  const bool useMaps = ( lithoMap1 && ! lithoMap1->isConstant ()) ||
                       ( lithoMap2 && ! lithoMap2->isConstant ()) ||
                       ( lithoMap3 && ! lithoMap3->isConstant ());

  bool undefinedMapValue = false;
  return formation->getLithologyFromStratTable( undefinedMapValue, useMaps, i, j, lithoMap1, lithoMap2, lithoMap3, lithoName1, lithoName2, lithoName3);
}

double DerivedPropertyDriller::calculateProperty( const std::string& propertyName ) const
{
  unsigned int i, j;
  double x = getXCoord( m_record );
  double y = getYCoord( m_record );
  const GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( m_dataDriller->getProjectHandle().findFormation(getFormationName( m_record ) ) );

  if ( !m_dataDriller->getGridLowResolution()->getGridPoint( x, y, i, j ) ) throw RecordException( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );


  if ( !formation )
  {
    const DataAccess::Interface::Surface* surface = m_dataDriller->getProjectHandle().findSurface( getSurfaceName( m_record ) );
    if ( surface )
    {
      formation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( surface->getBottomFormation() );
    }
    else
    {
      double iDouble, jDouble;
      if ( !m_dataDriller->getGridLowResolution()->getGridPoint( x, y, iDouble, jDouble ) ) throw RecordException( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );
      double z = getZCoord( m_record );
      formation = m_dataDriller->getFormation( iDouble, jDouble, z, m_snapshot );
    }
  }

  const GeoPhysics::CompoundLithology* lithology = getLithology( i, j, formation );
  if (!lithology)
  {
    return DefaultUndefinedScalarValue;
  }

  // Implemented Direct Derived Properties
  if ( propertyName == "Porosity" ||
       propertyName == "Permeability" ||
       propertyName == "LithoStaticPressure" || // For the basement it returns a DefaultUndefinedScalarValue
       propertyName == "DiffusivityVec2" ||
       propertyName == "Velocity" ||
       propertyName == "BrineViscosity" ||
       propertyName == "BrineDensity" ||
       propertyName == "BulkDensity" ||
       propertyName == "SonicSlowness" ||
       propertyName == "GammaRay" ||
       propertyName == "ThCondVec2" )   // For the basement it returns a DefaultUndefinedScalarValue
  {
    if ( formation )
    {
      std::unique_ptr<AbstractDerivedProperties::FormationPropertyCalculator> calculator( initializeCalculator( propertyName ) );
      return calculator->calculateAtPosition( formation, lithology, m_obtainedProperties );
    }
  }

  return DefaultUndefinedScalarValue;
}
