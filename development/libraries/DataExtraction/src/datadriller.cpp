//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "datadriller.h"

#include "ProjectHandle.h"
#include "SimulationDetails.h"

#include "DataMiningProjectHandle.h"
#include "DataMiningObjectFactory.h"

#include "errorhandling.h"
#include "hdfReadManager.h"
#include "trapPropertiesManager.h"
#include "GeoPhysicsFormation.h"
#include "derivedPropertyDriller.h"

namespace DataExtraction
{

DataDriller::DataDriller( const std::string& inputProjectFileName ) :
  DataExtractor(),
  m_obtained(),
  m_objectFactory( new DataAccess::Mining::ObjectFactory ),
  m_projectHandle( dynamic_cast<Mining::ProjectHandle*>( OpenCauldronProject( inputProjectFileName, m_objectFactory ) ) )
{
  database::Table* table = m_projectHandle->getTable( "DataMiningIoTbl" );

  if ( !table )
  {
    return;
  }

  m_obtained = std::vector<bool>(table->size(), true);

  m_gridHighResolution = m_projectHandle->getHighResolutionOutputGrid();
  m_gridLowResolution = m_projectHandle->getLowResolutionOutputGrid();
  m_projectHandle->startActivity ( "datadriller", m_gridHighResolution );
}

DataDriller::~DataDriller()
{
  if ( m_projectHandle )
  {
    m_projectHandle->finishActivity( false );
  }
  delete m_objectFactory;
}

DataAccess::Interface::ProjectHandle& DataDriller::getProjectHandle() const
{
    return *m_projectHandle;
}

const GeoPhysics::GeoPhysicsFormation* DataExtraction::DataDriller::getFormation(const double i, const double j, const double z, const Snapshot* snapshot) const
{
    HDFReadManager hdfReadManager( *m_projectHandle );
    const std::string snapshotFileName = m_projectHandle->getFullOutputDir() + "/" + snapshot->getFileName();
    hdfReadManager.openSnapshotFile( snapshotFileName );

    const std::string depthDataGroup = "/Depth";
    if ( !checkDataGroupInHDFFile( hdfReadManager, depthDataGroup, snapshotFileName ) )
    {
      return nullptr;
    }

    FormationList* myFormations = m_projectHandle->getFormations( snapshot, true );
    for ( const Formation* formation : *myFormations )
    {
      const std::string depthPropertyFormationName = depthDataGroup + "/" + formation->getMangledName();
      if ( !checkDataGroupInHDFFile( hdfReadManager, depthPropertyFormationName, snapshotFileName ) )
      {
          return nullptr;
      }
      DoubleVector depthVec = hdfReadManager.get3dCoordinatePropertyMatrix( {{ i, j}}, depthPropertyFormationName )[0];

      for ( int zi = 0; zi < depthVec.size() - 1; ++zi )
      {
        const double kf = getKfraction( depthVec[zi+1], depthVec[zi], z );
        if ( kf >= 0 && kf <= 1 )
        {
          return dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
        }
      }
    }
    delete myFormations;

    hdfReadManager.closeSnapshotFile();

    return nullptr;
}

bool DataDriller::allDataObtained()
{
  for ( const bool obtained : m_obtained )
  {
    if ( !obtained ) return false;
  }
  return true;
}

void DataDriller::run( const bool doCalculateTrapAndMissingProperties )
{
  performDirectDataDrilling();

  if ( doCalculateTrapAndMissingProperties && !allDataObtained() )
  {
    perform3DDataMining();
  }
}

void DataDriller::saveToFile( const std::string& outputProjectFileName )
{
  m_projectHandle->saveToFile( outputProjectFileName );
}

void DataDriller::perform3DDataMining()
{
  const DataAccess::Interface::SimulationDetails* simulationDetails = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );
  // If this table is not present the assume that the last
  // fastcauldron mode was not pressure mode.
  // This table may not be present because we are running c2e on an old
  // project, before this table was added.
  const bool coupledCalculation = ( simulationDetails != 0 ) && ( simulationDetails->getSimulatorMode () == "Overpressure" ||
                                                                  simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" ||
                                                                  simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" ||
                                                                  simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" ||
                                                                  simulationDetails->getSimulatorMode () == "CoupledDarcy" );


  m_projectHandle->initialise ( coupledCalculation );

  if ( !m_projectHandle->setFormationLithologies ( false, true ) )
  {
    std::cerr << " Cannot set lithologies." << std::endl;
  }

  m_projectHandle->initialiseLayerThicknessHistory ( coupledCalculation );
  CauldronDomain& domain = m_projectHandle->getCauldronDomain();
  DomainPropertyCollection* domainProperties = m_projectHandle->getDomainPropertyCollection();
  m_projectHandle->setFormationLithologies( false, false );

  database::Table* table = m_projectHandle->getTable( "DataMiningIoTbl" );
  if ( !table )
  {
    return;
  }

  DerivedProperties::DerivedPropertyManager propertyManager( *m_projectHandle );

  int recordIndex = -1;

  for ( database::Record* record : *table )
  {
    ++recordIndex;
    if ( m_obtained[recordIndex] )
    {
      continue; // Already read from the HDF file directly;
    }

    double value = DataAccess::Interface::DefaultUndefinedScalarValue;
    const DataAccess::Interface::Property* property = 0;

    try
    {
      const double snapshotTime = database::getTime (record);
      const DataAccess::Interface::Snapshot * snapshot = m_projectHandle->findSnapshot( snapshotTime, Interface::MAJOR | Interface::MINOR );

      const double x = database::getXCoord( record );
      const double y = database::getYCoord( record );
      const double z = database::getZCoord( record );

      const std::string& propertyName  = database::getPropertyName( record );
      const std::string& reservoirName = database::getReservoirName(record );
      const std::string& formationName = database::getFormationName(record );
      const std::string& surfaceName   = database::getSurfaceName(  record );

      property = m_projectHandle->findProperty( propertyName );

      // Get reservoir/trap property
      if ( reservoirName != "" )
      {
        const Interface::Reservoir * reservoir = m_projectHandle->findReservoir( reservoirName );
        TrapPropertiesManager trapPropertiesManager( *m_projectHandle, propertyManager );
        value = trapPropertiesManager.getTrapPropertyValue( property, snapshot, reservoir, x, y );
      }
      // Get property for X,Y,Z point or for Surface or Formation map
      else if ( z != DataAccess::Interface::DefaultUndefinedScalarValue || !surfaceName.empty() || !formationName.empty() )
      {
        domain.setSnapshot( snapshot, propertyManager );
        domainProperties->setSnapshot( snapshot );

        ElementPosition element;

        if ( z != DataAccess::Interface::DefaultUndefinedScalarValue ) // if z is given - look for the property at X,Y,Z point
        {
          domain.findLocation( x, y, z, element );
        }
        else if ( formationName != "" ) // z is not defined and formation is given - here we are having formation map property
        {
          const DataAccess::Interface::Formation * formation = m_projectHandle->findFormation( formationName );
          domain.findLocation( x, y, formation, element );
        }
        else if ( surfaceName != "") // if only surface name is given - look for surface property
        {
          const DataAccess::Interface::Surface * surface = m_projectHandle->findSurface (surfaceName);
          domain.findLocation( x, y, surface, element );
        }

        // calculate property value for specified position
        DomainProperty * domainProperty = domainProperties->getDomainProperty( property, propertyManager );
        if ( domainProperty )
        {
          domainProperty->initialise();
          value = domainProperty->compute( element );
        }
        }
    }
    catch( const RecordException & recordException )
    {
      m_obtained[recordIndex] = false;
      std::cerr << "Error in row " << recordIndex+1 << " of DataMiningIoTbl: " << recordException.what () << std::endl;
    }

    database::setValue( record, value );
    if ( property )
    {
      database::setPropertyUnit( record, property->getUnit() );
    }
  }
}

bool DataDriller::readPropertyFromHDF(const std::string& surfaceName, const std::string& formationName, const double x,
                                      const double y, const double z, const Interface::Snapshot* snapshot, const Interface::Property* property,
                                      double& value) const
{
  bool readFromHDF = false;
  if ( z != DataAccess::Interface::DefaultUndefinedScalarValue || !surfaceName.empty() || !formationName.empty() )
  {
    double i, j;
    if ( !m_gridLowResolution->getGridPoint( x, y, i, j ) ) throw RecordException( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );

    if ( z != DataAccess::Interface::DefaultUndefinedScalarValue ) // if z is given - look for the property at X,Y,Z point
    {
      readFromHDF = get3dPropertyFromHDF( i, j, z, property, snapshot, value);
    }

    if ( value == DataAccess::Interface::DefaultUndefinedScalarValue && surfaceName != "" )
    {
      const DataAccess::Interface::Surface * surface = m_projectHandle->findSurface (surfaceName);
      if ( !surface ) throw RecordException( "Unknown SurfaceName value: %", surfaceName );

      readFromHDF = get2dPropertyFromHDF( i, j, surface, nullptr, property, snapshot, value );
    }

    if ( value == DataAccess::Interface::DefaultUndefinedScalarValue && formationName != "" )
    {
      const DataAccess::Interface::Formation * formation = m_projectHandle->findFormation( formationName );
      if ( !formation ) throw RecordException( "Unknown FormationName value: %", formationName );

      readFromHDF = get2dPropertyFromHDF( i, j, nullptr, formation, property, snapshot, value );
    }

    if ( value == DataAccess::Interface::DefaultUndefinedScalarValue )
    {
      std::string formationMangledName = "";
      if ( formationName != "" )
      {
        formationMangledName = m_projectHandle->findFormation( formationName )->getMangledName();
      }
      else if ( surfaceName != "" )
      {
        formationMangledName = m_projectHandle->findSurface( surfaceName )->getBottomFormation()->getMangledName();
      }

      readFromHDF = get3dPropertyFromHDF( i, j, formationMangledName, true, property, snapshot, value );
    }
  }
  else
  {
    throw RecordException( "Illegal specification" );
  }
  return readFromHDF;
}

const Grid *DataDriller::getGridLowResolution() const
{
  return m_gridLowResolution;
}

void DataDriller::performDirectDataDrilling()
{
  database::Table* table = m_projectHandle->getTable( "DataMiningIoTbl" );
  int recordIndex = -1;
  DerivedPropertyDriller derivedPropertyDriller(this);
  for ( database::Record* record : *table )
  {
    ++recordIndex;
    double value = DataAccess::Interface::DefaultUndefinedScalarValue;
    const DataAccess::Interface::Property* property = nullptr;

    try
    {
      const double snapshotTime = database::getTime (record);
      if ( snapshotTime == DataAccess::Interface::DefaultUndefinedScalarValue ) throw RecordException( "Undefined Time value %:", snapshotTime );
      if ( snapshotTime < 0 )                                       throw RecordException( "Illegal snapshot time: %", snapshotTime );

      const DataAccess::Interface::Snapshot * snapshot = m_projectHandle->findSnapshot( snapshotTime, Interface::MAJOR | Interface::MINOR  );

      const double x = database::getXCoord( record );
      const double y = database::getYCoord( record );
      const double z = database::getZCoord( record );

      if ( x == DataAccess::Interface::DefaultUndefinedScalarValue ) throw RecordException ( "Undefined XCoord value: %", x );
      if ( y == DataAccess::Interface::DefaultUndefinedScalarValue ) throw RecordException ( "Undefined YCoord value: %", y );

      const std::string& propertyName  = database::getPropertyName( record );
      const std::string& reservoirName = database::getReservoirName(record );
      const std::string& formationName = database::getFormationName(record );
      const std::string& surfaceName   = database::getSurfaceName(  record );

      property = m_projectHandle->findProperty( propertyName );
      if ( !property ) throw RecordException( "Unknown PropertyName value: %", propertyName );

      // Skip in case of reservoir/trap property
      if ( reservoirName != "" )
      {
        const DataAccess::Interface::Reservoir * reservoir = m_projectHandle->findReservoir( reservoirName );
        if ( !reservoir ) throw RecordException( "Unknown ReservoirName value: %", reservoirName );

        m_obtained[recordIndex] = false;
        continue;
      }

      // Get property for X,Y,Z point or for Surface or Formation map
      if ( property->isPrimary() )
      {
        m_obtained[recordIndex] = readPropertyFromHDF(surfaceName, formationName, x, y, z, snapshot, property, value);
      }
      else
      {
        derivedPropertyDriller.setRecord(record);
        derivedPropertyDriller.setSnapshot(snapshot);

        m_obtained[recordIndex] = derivedPropertyDriller.run(value);

        if ( !m_obtained[recordIndex] )
        {
          m_obtained[recordIndex] = readPropertyFromHDF(surfaceName, formationName, x, y, z, snapshot, property, value);
        }
      }

    }
    catch( const RecordException & recordException )
    {
      std::cerr << "Error in row " << recordIndex + 1 << " of DataMiningIoTbl: " << recordException.what () << std::endl;
    }

    database::setValue( record, value );
    if ( property )
    {
      database::setPropertyUnit( record, property->getUnit() );
    }
  }
}

bool DataDriller::get2dPropertyFromHDF( const double i, const double j, const Surface* surface, const Formation* formation,
                                        const Property* property, const Snapshot* snapshot, double& value ) const
{
  HDFReadManager hdfReadManager( *m_projectHandle );
  hdfReadManager.openMapsFile( getMapsFileName( property->getCauldronName() ) );
  const std::string propertyFormationDataGroup = getPropertyFormationDataGroupName( formation, surface, property, snapshot );
  const DoubleVector values = hdfReadManager.get2dCoordinatePropertyVector( {{i, j}}, propertyFormationDataGroup );
  if ( !values.empty() )
  {
    hdfReadManager.closeMapsFile();
    value = values[0];
    return true;
  }

  hdfReadManager.closeMapsFile();
  return false;
}

bool DataDriller::checkDataGroupInHDFFile(HDFReadManager& hdfReadManager, const std::string& dataGroup, const std::string& snapshotFileName) const
{
  if ( !hdfReadManager.checkDataGroup( dataGroup ) )
  {
    hdfReadManager.closeSnapshotFile();
    return false;
  }

  return true;
}

bool DataDriller::get3dPropertyFromHDF( const double i, const double j, const double z,
                                          const DataAccess::Interface::Property* property,
                                          const DataAccess::Interface::Snapshot* snapshot,
                                          double& value) const
{
  if ( z <= 0.0 && property->getName() == "TwoWayTime" ) // in case of TwoWayTime property and z <= 0 set the value to 0
  {
    value = 0.0;
    return false;
  }

  HDFReadManager hdfReadManager( *m_projectHandle );
  const std::string snapshotFileName = m_projectHandle->getFullOutputDir() + "/" + snapshot->getFileName();
  hdfReadManager.openSnapshotFile( snapshotFileName );

  const std::string depthDataGroup = "/Depth";
  const std::string propertyDataGroup = "/" + property->getName();
  if ( !checkDataGroupInHDFFile( hdfReadManager, depthDataGroup, snapshotFileName ) ||
       !checkDataGroupInHDFFile( hdfReadManager, propertyDataGroup, snapshotFileName ) )
  {
    return false;
  }

  bool found = false;
  FormationList* myFormations = m_projectHandle->getFormations( snapshot, true );
  for ( const Formation* formation : *myFormations )
  {
    const std::string depthPropertyFormationName = depthDataGroup + "/" + formation->getMangledName();
    checkDataGroupInHDFFile( hdfReadManager, depthPropertyFormationName, snapshotFileName );
    DoubleVector depthVec = hdfReadManager.get3dCoordinatePropertyMatrix( {{ i, j}}, depthPropertyFormationName )[0];

    for ( int zi = 0; zi < depthVec.size() - 1; ++zi )
    {
      const double kf = getKfraction( depthVec[zi+1], depthVec[zi], z );
      if ( kf >= 0 && kf <= 1 )
      {
        const std::string propertyFormationDataGroup = propertyDataGroup + "/" + formation->getMangledName();
        if( !checkDataGroupInHDFFile( hdfReadManager, propertyFormationDataGroup, snapshotFileName ) )
        {
          return false;
        }
        DoubleVector propertyVec = hdfReadManager.get3dCoordinatePropertyMatrix( {{ i, j}}, propertyFormationDataGroup )[0];

        value = interpolate1d( propertyVec[zi+1], propertyVec[zi], kf );
        found = true;
        break;
      }
    }
    if ( found ) break;
  }
  delete myFormations;

  hdfReadManager.closeSnapshotFile();

  return found;
}

bool DataDriller::get3dPropertyFromHDF( const double i, const double j,
                                          const std::string & mangledName,
                                          const bool isSurfaceTop,
                                          const DataAccess::Interface::Property * property,
                                          const DataAccess::Interface::Snapshot * snapshot, double& value ) const
{
  HDFReadManager hdfReadManager( *m_projectHandle );
  const std::string snapshotFileName = m_projectHandle->getFullOutputDir() + "/" + snapshot->getFileName();
  hdfReadManager.openSnapshotFile( snapshotFileName );

  const std::string propertyDataGroup = "/" + property->getName();
  const std::string propertyFormationDataGroup = propertyDataGroup + "/" + mangledName;
  if ( !checkDataGroupInHDFFile( hdfReadManager, propertyDataGroup, snapshotFileName ) ||
       !checkDataGroupInHDFFile( hdfReadManager, propertyFormationDataGroup, snapshotFileName) ||
       mangledName == "")
  {
       return false;
  }

  DoubleVector propertyVec = hdfReadManager.get3dCoordinatePropertyMatrix( {{ i, j}}, propertyFormationDataGroup )[0];

  if (!propertyVec.empty())
  {
    value = (isSurfaceTop ? propertyVec.back() : propertyVec.front());
  }

  hdfReadManager.closeSnapshotFile();

  return true;
}

double DataDriller::interpolate1d( const double u, const double l, const double k) const
{
  return u + k * (l - u);
}

double DataDriller::getKfraction( const double u, const double  l, const double v) const
{
  if ( std::fabs( l - u ) < 1e-10)
  {
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }
  return ( v - u )/(l - u);
}

} // namespace DataExtraction
