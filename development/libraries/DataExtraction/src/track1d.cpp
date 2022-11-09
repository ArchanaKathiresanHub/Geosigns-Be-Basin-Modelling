//
// Copyright ( C ) 2013-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "track1d.h"

#include "Formation.h"
#include "Grid.h"
#include "SimulationDetails.h"
#include "Snapshot.h"
#include "Surface.h"

#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include "database.h"

#include "DerivedPropertyManager.h"

#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"

#include "FormationMapOutputPropertyValue.h"
#include "FormationOutputPropertyValue.h"
#include "FormationSurfaceOutputPropertyValue.h"
#include "SurfaceOutputPropertyValue.h"

#include "hdfReadManager.h"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>

namespace DataExtraction
{

Track1d::Track1d( const std::string& inputProjectFileName ) :
  DataExtractor(),
  m_objectFactory( new GeoPhysics::ObjectFactory() ),
  m_projectHandle( dynamic_cast<GeoPhysics::ProjectHandle*>( OpenCauldronProject( inputProjectFileName, m_objectFactory ) ) ),
  m_snapshots(),
  m_formationSurfacePairs(),
  m_properties(),
  m_logicalCoordinatePairs(),
  m_realWorldCoordinatePairs()
{
  createFormationMaxKMap();
  m_grid = m_projectHandle->getLowResolutionOutputGrid();
  m_projectHandle->startActivity ( "track1d", m_grid, false, false );
}

Track1d::~Track1d()
{
  if ( m_projectHandle )
  {
    m_projectHandle->finishActivity( false );
  }
  delete m_objectFactory;
}

bool Track1d::isCorrect() const
{
  return ( m_projectHandle != nullptr );
}

void Track1d::createFormationMaxKMap()
{
  const Snapshot* zeroSnapshot = m_projectHandle->findSnapshot( 0 );
  const std::string outputPath = m_projectHandle->getFullOutputDir();
  const std::string zeroSnapShotRootGroup = outputPath + "/" + zeroSnapshot->getFileName();

  HDFReadManager hdfReadManager( *m_projectHandle );
  if ( !hdfReadManager.openSnapshotFile( zeroSnapShotRootGroup ) )
  {
    return;
  }
  const FormationList* zeroFormations = m_projectHandle->getFormations( zeroSnapshot, true );
  for ( const Formation* formation : *zeroFormations )
  {
    const std::string depthFormationDataSpace = "/Depth/" + formation->getMangledName();
    if( hdfReadManager.checkDataGroup( depthFormationDataSpace ) )
    {
      DoubleMatrix matrix = hdfReadManager.get3dCoordinatePropertyMatrix({{0,0}}, depthFormationDataSpace );
      m_formationMaxKMap[formation] = matrix[0].size();
    }
    else
    {
      std::cerr << "ERROR: Could not find data for depth property of formation " << formation->getName() << " at age " << zeroSnapshot->getTime() << std::endl;
      std::cerr << "       Skipping this formation!!" << std::endl;
      m_formationMaxKMap[formation] = 0;
    }
  }
  delete zeroFormations;
}

std::vector<unsigned int> Track1d::retrieveKs( const FormationSurface& formationSurfacePair ) const
{
  const Formation* formation = formationSurfacePair.first;
  const Surface* surface = formationSurfacePair.second;

  int kMin = 0;
  int kMax = m_formationMaxKMap.at(formation) - 1;

  std::vector<unsigned int> ks;
  if ( surface == formation->getTopSurface() )
  {
    ks.push_back( kMax );
  }
  else if ( surface == formation->getBottomSurface() )
  {
    ks.push_back( kMin );
  }
  else
  {
    for ( int k = kMax; k >= kMin; --k )
    {
      ks.push_back(k);
    }
  }
  return ks;
}

void Track1d::acquireCoordinatePairs( const DoublePairVector& realWorldCoordinatePairs, const DoublePairVector& logicalCoordinatePairs )
{
  for ( const DoublePair& coordinatePair : realWorldCoordinatePairs )
  {
    double i, j;
    if ( !m_grid->getGridPoint( coordinatePair.first, coordinatePair.second, i, j ) )
    {
      std::cerr << "illegal coordinate pair: ( " << coordinatePair.first << ", " << coordinatePair.second << " )" << std::endl;
      continue;
    }
    m_realWorldCoordinatePairs.push_back( coordinatePair );
  }

  // convert i,j pairs into x,y pairs
  for ( const DoublePair& coordinatePair : logicalCoordinatePairs )
  {
    double x, y;
    if ( !m_grid->getPosition( coordinatePair.first, coordinatePair.second, x, y ) )
    {
      std::cerr << "illegal ( i,j ) coordinate pair: ( " << coordinatePair.first << ", " << coordinatePair.second << " )" << std::endl;
      continue;
    }

    m_realWorldCoordinatePairs.push_back( DoublePair( x, y ) );
  }

  for ( const DoublePair& coordinatePair : m_realWorldCoordinatePairs )
  {
    double i, j;
    m_grid->getGridPoint( coordinatePair.first, coordinatePair.second, i, j );
    m_logicalCoordinatePairs.push_back( DoublePair( i, j ) );
  }
}

void Track1d::acquireSnapshots( const DoubleVector& ages )
{
  if ( ages.size() == 0 )
  {
    return;
  }

  double firstAge = -1;
  double secondAge = -1;
  for ( const double age : ages )
  {
    if ( age >= 0 )
    {
      if ( firstAge < 0 )
        firstAge = age;
      else
        secondAge = age;
    }
    else
    {
      if ( secondAge < 0 )
      {
        if ( firstAge >= 0 )
        {
          const Snapshot* snapshot = m_projectHandle->findSnapshot( firstAge );
          if ( snapshot ) m_snapshots.push_back( snapshot );
        }
      }
      else
      {
        if ( firstAge >= 0 )
        {
          if ( firstAge > secondAge )
          {
            std::swap(firstAge, secondAge);
          }

          SnapshotList* allSnapshots = m_projectHandle->getSnapshots();
          for ( const Snapshot* snapshot : *allSnapshots )
          {
            if ( snapshot && snapshot->getTime() >= firstAge && snapshot->getTime() <= secondAge )
            {
              m_snapshots.push_back( snapshot );
            }
          }
          delete allSnapshots;
        }
      }
      firstAge = secondAge = -1;
    }
  }

  std::sort( m_snapshots.begin(), m_snapshots.end(), snapshotSorter );

  SnapshotList::iterator firstObsolete = std::unique( m_snapshots.begin(), m_snapshots.end(), snapshotIsEqual );
  m_snapshots.erase( firstObsolete, m_snapshots.end() );
}

void Track1d::acquireProperties( const StringVector& propertyNamesUser, const bool all2Dproperties, const bool all3Dproperties )
{
  StringVector propertyNames = propertyNamesUser;
  if ( propertyNames.empty() )
  {
    propertyNames = getPropertyNames( all2Dproperties, all3Dproperties );
  }

  DerivedProperties::DerivedPropertyManager propertyManager( *m_projectHandle );

  const Property* depthProperty = m_projectHandle->findProperty( "Depth" );
  assert( depthProperty );
  m_properties.push_back( depthProperty );

  for ( const std::string& propertyName : propertyNames )
  {

    const Property* property = m_projectHandle->findProperty( propertyName );
    bool isComputable = false;

    if ( property == 0 )
    {
      std::cerr << "Could not find property named '" << propertyName << "'" << std::endl;
      continue;
    }

    if ( ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
           property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and
         propertyManager.formationPropertyIsComputable ( property ) )
    {
      isComputable = true;
    }
    else if ( ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
                property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and
              propertyManager.surfacePropertyIsComputable ( property ) )
    {
      isComputable = true;
    }
    else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
              propertyManager.formationSurfacePropertyIsComputable ( property ) )
    {
      isComputable = true;
    }
    else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and
              propertyManager.formationMapPropertyIsComputable ( property ) )
    {
      isComputable = true;
    }

    if ( isComputable ) {
      m_properties.push_back( property );
    } else {
      std::cerr << "Could not find calculator for property named '" << propertyName << "'" << std::endl;
    }

  }
}

void Track1d::acquireFormationSurfacePairs( const StringVector& topSurfaceFormationNames,
                                            const StringVector& bottomSurfaceFormationNames,
                                            const StringVector& formationNames,
                                            const StringVector& formationSurfaceNames,
                                            const bool basement)
{
  acquireFormationSurfaces( m_formationSurfacePairs, topSurfaceFormationNames, true );
  acquireFormationSurfaces( m_formationSurfacePairs, bottomSurfaceFormationNames, false );
  acquireFormations( m_formationSurfacePairs, formationNames );
  acquireSurfaces( m_formationSurfacePairs, formationSurfaceNames );

  if ( m_formationSurfacePairs.empty() )
  {
    FormationList* formations = m_projectHandle->getFormations( m_projectHandle->findSnapshot( 0 ), basement );
    for ( const Formation* formation : *formations )
    {
      m_formationSurfacePairs.push_back( FormationSurface( formation, 0 ) );
    }
    delete formations;
  }
}

void Track1d::acquireFormationSurfaces( FormationSurfaceVector& formationSurfacePairs, const StringVector& formationNames, bool useTop )
{
  for ( const std::string& formationName : formationNames )
  {
    const Formation* formation = m_projectHandle->findFormation( formationName );
    if ( !formation )
    {
      std::cerr << "Could not find formation named '" << formationName << "'" << std::endl;
      continue;
    }

    const Surface* surface = useTop ? formation->getTopSurface() : formation->getBottomSurface();
    if ( !surface )
    {
      std::cerr << "Could not find " << ( useTop ? "Top " : "Bottom" ) << " surface for formation named '" << formationName << "'" << std::endl;
      continue;
    }
    formationSurfacePairs.push_back( FormationSurface( formation, surface ) );
  }
}

void Track1d::acquireFormations( FormationSurfaceVector& formationSurfacePairs, const StringVector& formationNames )
{
  for ( const std::string& formationName : formationNames )
  {
    const Formation* formation = m_projectHandle->findFormation( formationName );
    if ( !formation )
    {
      std::cerr << "Could not find formation named '" << formationName << "'" << std::endl;
      continue;
    }

    formationSurfacePairs.push_back( FormationSurface( formation, 0 ) );
  }
}

void Track1d::acquireSurfaces(FormationSurfaceVector& formationSurfacePairs, const StringVector& formationSurfaceNames )
{
  for ( const std::string& surfaceName : formationSurfaceNames )
  {
    char* formationName;
    char* formationSurfaceName;
    char surfaceNamePtr[ 256 ];
    std::strcpy( surfaceNamePtr, surfaceName.c_str() );

    if ( ParseUtilities::splitString( surfaceNamePtr, ':', formationName, formationSurfaceName ) == false ) continue;
    if ( formationName == 0 || std::strlen( formationName ) == 0 ) continue;
    if ( formationSurfaceName == 0 || std::strlen( formationSurfaceName ) == 0 ) continue;

    const Formation* formation = m_projectHandle->findFormation( formationName );
    if ( !formation )
    {
      std::cerr << "Could not find formation named '" << surfaceName << "'" << std::endl;
      continue;
    }

    const Surface* surface = m_projectHandle->findSurface( formationSurfaceName );
    if ( !surface )
    {
      std::cerr << "Could not find surface named '" << formationSurfaceName << "'" << std::endl;
      continue;
    }
    formationSurfacePairs.push_back( FormationSurface( formation, surface ) );
  }
}

bool Track1d::snapshotIsEqual( const Snapshot* snapshot1, const Snapshot* snapshot2 )
{
  return std::fabs( snapshot1->getTime() - snapshot2->getTime() ) < std::numeric_limits<double>::epsilon();
}

SnapshotFormationOutputPropertyValueMap Track1d::allOutputPropertyValues() const
{
  return m_allOutputPropertyValues;
}

bool Track1d::snapshotSorter( const Snapshot* snapshot1, const Snapshot* snapshot2 )
{
  return snapshot1->getTime() > snapshot2->getTime();
}

StringVector Track1d::getPropertyNames( bool all2Dproperties, bool all3Dproperties ) const
{
  DerivedProperties::DerivedPropertyManager propertyManager( *m_projectHandle );
  StringVector propertyNames;
  const PropertyList* allProperties = m_projectHandle->getProperties( true );
  for ( const Interface::Property* property : *allProperties )
  {
    bool addIt = false;
    switch ( property->getPropertyAttribute () )
    {
      case DataModel::CONTINUOUS_3D_PROPERTY:
        addIt = ( all2Dproperties && propertyManager.surfacePropertyIsComputable ( property ) ) ||
            ( all3Dproperties && propertyManager.formationPropertyIsComputable ( property ) );
        break;
      case DataModel::SURFACE_2D_PROPERTY:
        addIt = all2Dproperties && propertyManager.surfacePropertyIsComputable ( property );
        break;
      case DataModel::DISCONTINUOUS_3D_PROPERTY:
        addIt = ( all2Dproperties && propertyManager.formationSurfacePropertyIsComputable ( property ) ) ||
            ( all3Dproperties && propertyManager.formationPropertyIsComputable ( property ) );
        break;
      case DataModel::FORMATION_2D_PROPERTY:
        addIt = all2Dproperties && propertyManager.formationMapPropertyIsComputable ( property );
        break;
      default:
        break;
    }

    if ( addIt )
    {
      propertyNames.push_back( property->getName() );
    }
  }
  delete allProperties;
  return propertyNames;
}

SnapshotList* Track1d::getSnapshots() const
{
  return m_projectHandle->getSnapshots();
}

FormationList* Track1d::getFormationsSnapshot0() const
{
  return m_projectHandle->getFormations( m_projectHandle->findSnapshot( 0 ), true );
}

bool Track1d::run( const bool lean )
{
  readDataFromHDFfiles();

  return ( lean || calculateDerivedProperties() );
}

DerivedProperties::OutputPropertyValuePtr Track1d::allocateOutputProperty( DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                           const DataModel::AbstractProperty* property,
                                                                           const DataModel::AbstractSnapshot* snapshot,
                                                                           const FormationSurface& formationItem )
{
  DerivedProperties::OutputPropertyValuePtr outputProperty;

  const Interface::Formation* formation = formationItem.first;
  const Interface::Surface* topSurface = 0;
  const Interface::Surface* bottomSurface = 0;

  if ( ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
         property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and
       propertyManager.formationPropertyIsComputable ( property, snapshot, formation ) )
  {
    outputProperty = DerivedProperties::OutputPropertyValuePtr ( new DerivedProperties::FormationOutputPropertyValue ( propertyManager, property, snapshot, formation ) );
  }

  if ( outputProperty == 0 ) {


    if ( formation != 0 and formationItem.second != 0  ) {

      if ( formation->getTopSurface () != 0 and formationItem.second == formation->getTopSurface () ) {
        topSurface = formation->getTopSurface ();
      } else if ( formation->getBottomSurface () != 0 and formationItem.second == formation->getBottomSurface () ) {
        bottomSurface = formation->getBottomSurface ();
      }

    }

    // First check if the surface property is computable
    if ( ( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
           property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and
         ( ( topSurface != 0    and propertyManager.surfacePropertyIsComputable ( property, snapshot, topSurface ) ) or
           ( bottomSurface != 0 and propertyManager.surfacePropertyIsComputable ( property, snapshot, bottomSurface ) ) ) )
    {

      if ( topSurface != 0 ) {
        outputProperty = DerivedProperties::OutputPropertyValuePtr ( new DerivedProperties::SurfaceOutputPropertyValue ( propertyManager, property, snapshot, topSurface ) );
      } else if ( bottomSurface != 0 ) {
        outputProperty = DerivedProperties::OutputPropertyValuePtr ( new DerivedProperties::SurfaceOutputPropertyValue ( propertyManager, property, snapshot, bottomSurface ) );
      }

    }
    // Next check if the formation-surface property is computable
    else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
              ( ( topSurface != 0    and propertyManager.formationSurfacePropertyIsComputable ( property, snapshot, formation, topSurface ) ) or
                ( bottomSurface != 0 and propertyManager.formationSurfacePropertyIsComputable ( property, snapshot, formation, bottomSurface ) ) ) )

    {

      if ( topSurface != 0 ) {
        outputProperty = DerivedProperties::OutputPropertyValuePtr ( new DerivedProperties::FormationSurfaceOutputPropertyValue ( propertyManager, property, snapshot, formation, topSurface ) );
      } else if ( bottomSurface != 0 ) {
        outputProperty = DerivedProperties::OutputPropertyValuePtr ( new DerivedProperties::FormationSurfaceOutputPropertyValue ( propertyManager, property, snapshot, formation, bottomSurface ) );
      }

    }
    // Finally check if the formation-map property is computable
    else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and
              propertyManager.formationMapPropertyIsComputable ( property, snapshot, formation ) )
    {
      outputProperty = DerivedProperties::OutputPropertyValuePtr ( new DerivedProperties::FormationMapOutputPropertyValue ( propertyManager, property, snapshot, formation ) );
    }

  }

  return outputProperty;
}

void Track1d::readDataFromHDFfiles()
{
  HDFReadManager hdfReadManager( *m_projectHandle );

  for ( const Snapshot* snapshot : m_snapshots )
  {
    const std::string snapShotRootGroup = m_projectHandle->getFullOutputDir() + "/" + snapshot->getFileName();
    if ( !hdfReadManager.openSnapshotFile( snapShotRootGroup ) )
    {
      continue;
    }

    for ( const Property* property : m_properties )
    {
      const std::string propertyDataSpace = "/" + property->getName();
      const bool data3dAvailable = hdfReadManager.checkDataGroup( propertyDataSpace );

      for ( const FormationSurface& formationSurfacePair : m_formationSurfacePairs )
      {
        const Formation* formation = formationSurfacePair.first;

        if ( data3dAvailable )
        {
          const std::string propertyFormationDataSpace = propertyDataSpace + "/" + formation->getMangledName();
          DoubleMatrix matrix = hdfReadManager.get3dCoordinatePropertyMatrix( m_logicalCoordinatePairs, propertyFormationDataSpace );

          for ( int coordPair = 0; coordPair < matrix.size(); ++coordPair )
          {
            for ( int k = 0; k < matrix[coordPair].size(); ++k )
            {
              m_allOutputPropertyValues[ snapshot ][ formation ][ coordPair ][ property ][ k ] = matrix[coordPair][k];
            }
          }
        }
        else if ( property->getPropertyAttribute() == DataModel::FORMATION_2D_PROPERTY ||
                  property->getPropertyAttribute() == DataModel::SURFACE_2D_PROPERTY )
        {
          hdfReadManager.openMapsFile( getMapsFileName( property->getCauldronName() ) );

          const std::string propertyFormationDataGroup = getPropertyFormationDataGroupName( formation, formationSurfacePair.second, property, snapshot ) ;
          const DoubleVector propertyData = hdfReadManager.get2dCoordinatePropertyVector( m_logicalCoordinatePairs, propertyFormationDataGroup );

          for ( int coordPair = 0; coordPair < propertyData.size(); ++coordPair )
          {
            for ( unsigned int k : retrieveKs( formationSurfacePair ) )
            {
              m_allOutputPropertyValues[ snapshot ][ formation ][ coordPair ][ property ][ k ] = propertyData[coordPair];
            }
          }
          hdfReadManager.closeMapsFile();
        }
      }
    }

    hdfReadManager.closeSnapshotFile();
  }
}

bool Track1d::calculateDerivedProperties()
{
  bool coupledCalculation = false;

  const Interface::SimulationDetails* simulationDetails = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );
  if ( simulationDetails )
  {
    // If this table is not present the assume that the last
    // fastcauldron mode was not pressure mode.
    // This table may not be present because we are running c2e on an old
    // project, before this table was added.
    coupledCalculation = simulationDetails->getSimulatorMode () == "Overpressure" or
        simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" or
        simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" or
        simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" or
        simulationDetails->getSimulatorMode () == "CoupledDarcy";
  }

  try
  {
    if ( !m_projectHandle->initialise ( coupledCalculation ) ||
         !m_projectHandle->setFormationLithologies ( false, true ) ||
         !m_projectHandle->initialiseLayerThicknessHistory ( coupledCalculation ) )
    {
      std::cerr << "Could not initialize project handle" << std::endl;
      return false;
    }
  }
  catch ( std::exception& e )
  {
    std::cerr << e.what() << std::endl;
    return false;
  }

  DerivedProperties::DerivedPropertyManager propertyManager( *m_projectHandle );

  for ( const Snapshot* snapshot : m_snapshots )
  {
    for ( const FormationSurface& formationSurfacePair : m_formationSurfacePairs )
    {
      const Formation* formation = formationSurfacePair.first;

      for ( const Property* property : m_properties )
      {
        if ( m_allOutputPropertyValues[ snapshot ][ formation ][ 0 ].empty() ||
             m_allOutputPropertyValues[ snapshot ][ formation ][ 0 ][property].empty() )
        {
          DerivedProperties::OutputPropertyValuePtr outputProperty = allocateOutputProperty( propertyManager, property, snapshot, formationSurfacePair );
          if ( outputProperty )
          {
            int coordPair = 0;
            for ( const DoublePair& coordinatePair : m_logicalCoordinatePairs )
            {
              for ( unsigned int k : retrieveKs( formationSurfacePair ) )
              {
                const double value = outputProperty->getValue( coordinatePair.first, coordinatePair.second, k );
                m_allOutputPropertyValues[ snapshot ][ formation ][ coordPair ][ property ][ k ] = value;
              }
              ++coordPair;
            }
          }
        }
      }
    }
    propertyManager.removeProperties( snapshot ); // Clear memory used
  }

  return true;
}

ProjectHandle& Track1d::getProjectHandle() const
{
  return *m_projectHandle;
}

void Track1d::writeOutputStream( std::ostream& outputStream,
                                 const bool history )
{
  outputStream << "X(m),Y(m),I,J,Age(Ma),Formation,Surface,LayerIndex";

  for ( const Property* property : m_properties )
  {
    outputStream << "," << property->getName() << "(" << property->getUnit() << ")";
  }
  outputStream << std::endl;

  for ( int coordPair = 0; coordPair < m_realWorldCoordinatePairs.size(); ++coordPair )
  {
    const double x = m_realWorldCoordinatePairs[coordPair].first;
    const double y = m_realWorldCoordinatePairs[coordPair].second;
    const double i = m_logicalCoordinatePairs[coordPair].first;
    const double j = m_logicalCoordinatePairs[coordPair].second;

    if ( history )
    {
      for ( const FormationSurface& formationSurfacePair : m_formationSurfacePairs )
      {
        const Formation* formation = formationSurfacePair.first;
        const Surface* surface = formationSurfacePair.second;

        const std::vector<unsigned int> ks = retrieveKs( formationSurfacePair );
        for ( unsigned int k : ks )
        {
          const std::string formationSurfaceName = getFormationSurfaceName( ks, k, formation, surface);
          for ( const Snapshot* snapshot : m_snapshots )
          {
            if ( m_allOutputPropertyValues[ snapshot ][ formation ][coordPair][ m_properties[ 0 ]].empty() ) continue;

            outputSnapshotFormationData( outputStream, x, y, i, j, snapshot, formation->getName(), formationSurfaceName,
                                         m_allOutputPropertyValues[snapshot][formation][coordPair], k );
          }
        }
      }
    }
    else
    {
      for ( const Snapshot* snapshot : m_snapshots )
      {
        for ( const FormationSurface& formationSurfacePair : m_formationSurfacePairs )
        {
          const Formation* formation = formationSurfacePair.first;
          const Surface* surface = formationSurfacePair.second;

          if ( m_allOutputPropertyValues[ snapshot ][ formation ][coordPair][ m_properties[ 0 ]].empty() ) continue;

          const std::vector<unsigned int> ks = retrieveKs( formationSurfacePair );
          for ( unsigned int k : ks )
          {
            const std::string formationSurfaceName = getFormationSurfaceName( ks, k, formation, surface);

            outputSnapshotFormationData( outputStream, x, y, i, j, snapshot, formation->getName(), formationSurfaceName,
                                         m_allOutputPropertyValues[snapshot][formation][coordPair], k );
          }
        }
      }
    }
  }
}

std::vector<double> Track1d::getData(const std::string& propertyName)
{
  std::vector<double> propertyData;
  for ( int coordPair = 0; coordPair < m_realWorldCoordinatePairs.size(); ++coordPair )
  {
    for ( const Snapshot* snapshot : m_snapshots )
    {
      for ( const FormationSurface& formationSurfacePair : m_formationSurfacePairs )
      {
        const Formation* formation = formationSurfacePair.first;
        const Surface* surface = formationSurfacePair.second;

        if ( m_allOutputPropertyValues[ snapshot ][ formation ][coordPair][ m_properties[ 0 ]].empty() ) continue;

        const std::vector<unsigned int> ks = retrieveKs( formationSurfacePair );
        for ( unsigned int k : ks )
        {
          const std::string formationSurfaceName = getFormationSurfaceName( ks, k, formation, surface);
          for ( const Property* property : m_properties )
          {
            if (property->getName() == propertyName)
            {
              propertyData.push_back(m_allOutputPropertyValues[snapshot][formation][coordPair][property][k]);
            }
          }
        }
      }
    }
  }

  return propertyData;
}

void Track1d::outputSnapshotFormationData( std::ostream& outputStream, double x, double y, double i, double j,
                                           const Snapshot* snapshot, const std::string& formationName, const std::string& formationSurfaceName,
                                           OutputPropertyValueMap& allOutputPropertyValues,
                                           unsigned int k )
{
  std::streamsize oldPrecision = outputStream.precision();
  outputStream << std::setprecision( 12 ) << x << "," << y << std::setprecision( oldPrecision );
  outputStream << "," << i << "," << j << ",";
  outputStream << std::setprecision( 9 ) << snapshot->getTime() << std::setprecision( oldPrecision );
  outputStream << "," << formationName;
  outputStream << "," << formationSurfaceName;

  outputStream << "," << k;

  for ( const Property* property : m_properties )
  {
    outputStream << ",";

    if ( !allOutputPropertyValues[ property ].empty() )
    {
      const double value = allOutputPropertyValues[ property ][k];

      if ( value != DataAccess::Interface::DefaultUndefinedMapValue )
      {
        outputStream << value;
      }
      else
      {
        outputStream << " ";
      }
    }
    else
    {
      outputStream << " ";
    }
  }
  outputStream << std::endl;
}

std::string Track1d::getFormationSurfaceName( const std::vector<unsigned int>& ks, unsigned int k, const Formation* formation, const Surface* surface )
{
  if (surface)
  {
    return surface->getName();
  }
  else if ( k == ks.front() )
  {
    return formation->getTopSurfaceName();
  }
  else if ( k == ks.back() )
  {
    return formation->getBottomSurfaceName();
  }
  return "";
}

void Track1d::doListProperties()
{
  std::cout << "Available 3D output properties are: ";

  for ( const std::string& propertyName : getPropertyNames( false, true ) )
  {
    std::cout << propertyName << "  ";
  }

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Available 2D output properties are: ";

  for ( const std::string& propertyName : getPropertyNames( true, false ) )
  {
    std::cout << propertyName << "  ";
  }

  std::cout << std::endl;
  std::cout << std::endl;
}

void Track1d::doListSnapshots()
{
  std::cout << std::endl;
  SnapshotList* mySnapshots = getSnapshots();
  bool first = true;
  std::cout.precision ( 8 );
  std::cout << "Available snapshots are: ";
  for ( const Snapshot* snapshot : *mySnapshots )
  {
    if (first)
		  first = false;
	  else
		  std::cout << ",";
    std::streamsize oldPrecision = std::cout.precision();
    std::cout << std::setprecision( 9 ) << snapshot->getTime() << std::setprecision( oldPrecision );
  }
  std::cout << std::endl;
  delete mySnapshots;
}

void Track1d::doListStratigraphy()
{
  std::cout << std::endl;
  FormationList* myFormations = getFormationsSnapshot0();
  if ( myFormations->empty() )
  {
    return;
  }
  std::cout << "Stratigraphy: ";
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "\t" << myFormations->front()->getTopSurface()->getName() << " ( "
            << myFormations->front()->getTopSurface()->getSnapshot()->getTime() << " Ma )" << std::endl;

  bool arrivedAtBasement = false;
  for ( const Formation* formation : *myFormations )
  {
    if ( formation->kind() == BASEMENT_FORMATION && arrivedAtBasement == false )
    {
      arrivedAtBasement = true;
      std::cout << "Next formations and surfaces belong to the basement and only produce output when used with '-basement'" << std::endl;
    }

    std::cout << "\t\t" << formation->getName() << std::endl;
    const Surface* bottomSurface = formation->getBottomSurface();
    if ( bottomSurface )
    {
      std::cout << "\t" << bottomSurface->getName();
      if ( bottomSurface->getSnapshot() )
      {
        std::cout << " ( " << bottomSurface->getSnapshot()->getTime() << " Ma )";
      }
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;
  delete myFormations;
}

} // namespace DataExtraction
