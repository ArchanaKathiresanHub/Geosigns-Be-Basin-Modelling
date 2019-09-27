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

namespace DataExtraction
{

DataDriller::DataDriller( const std::string& inputProjectFileName ) :
  DataExtractor(),
  m_objectFactory( new DataAccess::Mining::ObjectFactory ),
  m_projectHandle( dynamic_cast<Mining::ProjectHandle*>( OpenCauldronProject( inputProjectFileName, "r", m_objectFactory ) ) ),
  m_readFromHDF()
{
  m_gridHighResolution = m_projectHandle->getHighResolutionOutputGrid();
  m_gridLowResolution = m_projectHandle->getLowResolutionOutputGrid();
  m_projectHandle->startActivity ( "datadriller", m_gridHighResolution );
}

DataDriller::~DataDriller()
{
  if ( m_projectHandle )
  {
    m_projectHandle->finishActivity( false );
    delete m_projectHandle;
  }
  delete m_objectFactory;
}

DataAccess::Interface::ProjectHandle* DataDriller::getProjectHandle() const
{
  return m_projectHandle;
}

bool DataDriller::allDataReadFromHDF()
{
  for ( const bool readDataFromHDF : m_readFromHDF )
  {
    if ( readDataFromHDF ) return false;
  }
  return true;
}

void DataDriller::run( const bool doCalculateTrapAndMissingProperties )
{
  readDataFromHDF();
  if ( doCalculateTrapAndMissingProperties && !allDataReadFromHDF() )
  {
    calculateTrapAndMissingProperties();
  }
}

void DataDriller::saveToFile( const std::string& outputProjectFileName )
{
  m_projectHandle->saveToFile( outputProjectFileName );
}

void DataDriller::calculateTrapAndMissingProperties()
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

  DerivedProperties::DerivedPropertyManager propertyManager( m_projectHandle );

  int recordIndex = -1;

  for ( database::Record* record : *table )
  {
    ++recordIndex;
    if ( m_readFromHDF[recordIndex] )
    {
      continue; // Already read from the HDF file directly;
    }

    double value = DataAccess::Interface::DefaultUndefinedScalarValue;
    const DataAccess::Interface::Property* property = 0;

    try
    {
      const double snapshotTime = database::getTime (record);
      const DataAccess::Interface::Snapshot * snapshot = m_projectHandle->findSnapshot( snapshotTime );

      const double x = database::getXCoord( record );
      const double y = database::getYCoord( record );
      const double z = database::getZCoord( record );

      const string& propertyName  = database::getPropertyName( record );
      const string& reservoirName = database::getReservoirName(record );
      const string& formationName = database::getFormationName(record );
      const string& surfaceName   = database::getSurfaceName(  record );

      property = m_projectHandle->findProperty( propertyName );

      // Get reservoir/trap property
      if ( reservoirName != "" )
      {
        const DataAccess::Interface::Reservoir * reservoir = m_projectHandle->findReservoir( reservoirName );
        TrapPropertiesManager trapPropertiesManager( m_projectHandle, propertyManager );
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
        else if ( surfaceName != "" && formationName == "" ) // if only surface name is given - look for surface property
        {
          const DataAccess::Interface::Surface * surface = m_projectHandle->findSurface (surfaceName);
          domain.findLocation( x, y, surface, element );
        }
        else if ( formationName != "" && surfaceName == "" ) // z is not defined and formation is given - here we are having formation map property
        {
          const DataAccess::Interface::Formation * formation = m_projectHandle->findFormation( formationName );
          domain.findLocation( x, y, formation, element );
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
      cerr << "Error in row " << recordIndex+1 << " of DataMiningIoTbl: " << recordException.what () << endl;
    }

    database::setValue( record, value );
    if ( property )
    {
      database::setPropertyUnit( record, property->getUnit() );
    }
  }
}

void DataDriller::readDataFromHDF()
{
  database::Table* table = m_projectHandle->getTable( "DataMiningIoTbl" );

  if ( !table )
  {
    return;
  }

  m_readFromHDF = std::vector<bool>(table->size(), true);

  int recordIndex = -1;
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

      const DataAccess::Interface::Snapshot * snapshot = m_projectHandle->findSnapshot( snapshotTime );

      const double x = database::getXCoord( record );
      const double y = database::getYCoord( record );
      const double z = database::getZCoord( record );

      if ( x == DataAccess::Interface::DefaultUndefinedScalarValue ) throw RecordException ( "Undefined XCoord value: %", x );
      if ( y == DataAccess::Interface::DefaultUndefinedScalarValue ) throw RecordException ( "Undefined YCoord value: %", y );

      const string& propertyName  = database::getPropertyName( record );
      const string& reservoirName = database::getReservoirName(record );
      const string& formationName = database::getFormationName(record );
      const string& surfaceName   = database::getSurfaceName(  record );

      property = m_projectHandle->findProperty( propertyName );
      if ( !property ) throw RecordException( "Unknown PropertyName value: %", propertyName );

      if ( property->getName() == "Porosity" ||
           property->getName() == "Permeability"  ||
           property->getName() == "HorizontalPermeability" )
      {
        m_readFromHDF[recordIndex] = false;
        continue; // Not computed here due to inaccuracy of linear interpolation
      }

      // Get reservoir/trap property
      if ( reservoirName != "" )
      {
        const DataAccess::Interface::Reservoir * reservoir = m_projectHandle->findReservoir( reservoirName );
        if ( !reservoir ) throw RecordException( "Unknown ReservoirName value: %", reservoirName );

        m_readFromHDF[recordIndex] = false;
        continue; // Not computed here
      }
      // Get property for X,Y,Z point or for Surface or Formation map
      else if ( z != DataAccess::Interface::DefaultUndefinedScalarValue || !surfaceName.empty() || !formationName.empty() )
      {
        double i, j;
        if ( !m_gridLowResolution->getGridPoint( x, y, i, j ) ) throw RecordException( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );

        if ( z != DataAccess::Interface::DefaultUndefinedScalarValue ) // if z is given - look for the property at X,Y,Z point
        {
          value = get3dPropertyFromHDF( i, j, z, property, snapshot, recordIndex );
        }
        else if ( surfaceName != "" && formationName == "" ) // if only surface name is given - look for surface property
        {
          const DataAccess::Interface::Surface * surface = m_projectHandle->findSurface (surfaceName);
          if ( !surface ) throw RecordException( "Unknown SurfaceName value: %", surfaceName );

          value = get2dPropertyFromHDF( i, j, surface, nullptr, property, snapshot, recordIndex );
        }
        else if ( formationName != "" && surfaceName == "" ) // z is not defined and formation is given - here we are having formation map property
        {
          const DataAccess::Interface::Formation * formation = m_projectHandle->findFormation( formationName );
          if ( !formation ) throw RecordException( "Unknown FormationName value: %", formationName );

          // check for FORMATION MAP properties which are only allowed here:
          if ( !property->hasPropertyValues( DataAccess::Interface::FORMATION, snapshot, 0, formation, 0, DataAccess::Interface::MAP ) )
          {
            throw RecordException( "Volume properties unsupported for the FORMATION MAP property request: Z value is undefined and Formation name is specified" );
          }

          value = get2dPropertyFromHDF( i, j, nullptr, formation, property, snapshot, recordIndex );
        }
      }
      else if ( formationName != "" && surfaceName != "" ) { throw RecordException( "Use of FormationName together with SurfaceName is not yet implemented:" ); }
      else                                                 { throw RecordException( "Illegal specification" ); }
    }
    catch( const RecordException & recordException )
    {
      cerr << "Error in row " << recordIndex + 1 << " of DataMiningIoTbl: " << recordException.what () << endl;
    }

    database::setValue( record, value );
    if ( property )
    {
      database::setPropertyUnit( record, property->getUnit() );
    }
  }
}

double DataDriller::get2dPropertyFromHDF( const double i, const double j, const Surface* surface, const Formation* formation,
                                          const Property* property, const Snapshot* snapshot, const unsigned int recordIndex )
{
  HDFReadManager hdfReadManager( m_projectHandle );
  hdfReadManager.openMapsFile( getMapsFileName( property->getCauldronName() ) );
  const std::string propertyFormationDataGroup = getPropertyFormationDataGroupName( formation, surface, property, snapshot );
  const DoubleVector values = hdfReadManager.get2dCoordinatePropertyVector( {{i, j}}, propertyFormationDataGroup );
  if ( !values.empty() )
  {
    return values[0];
  }

  m_readFromHDF[recordIndex] = false;
  return DataAccess::Interface::DefaultUndefinedScalarValue;
}

double DataDriller::get3dPropertyFromHDF( const double i, const double j, const double z,
                                          const DataAccess::Interface::Property* property,
                                          const DataAccess::Interface::Snapshot* snapshot,
                                          const unsigned int recordIndex )
{
  if ( z <= 0.0 && property->getName() == "TwoWayTime" ) // in case of TwoWayTime property and z <= 0 set the value to 0
  {
    return 0.0;
  }

  HDFReadManager hdfReadManager( m_projectHandle );

  const std::string snapshotFileName = m_projectHandle->getFullOutputDir() + "/" + snapshot->getFileName();
  hdfReadManager.openSnapshotFile( snapshotFileName );

  auto checkDataGroup = [&]( const string& dataGroup )
  {
    if ( !hdfReadManager.checkDataGroup( dataGroup ) )
    {
      hdfReadManager.closeSnapshotFile();
      m_readFromHDF[recordIndex] = false;
      throw RecordException( "Data group " + dataGroup + " is not available in " + snapshotFileName );
    }
  };

  const string depthDataGroup = "/Depth";
  checkDataGroup( depthDataGroup );

  const string propertyDataGroup = "/" + property->getName();
  checkDataGroup( propertyDataGroup );

  double value = DataAccess::Interface::DefaultUndefinedScalarValue;
  bool found = false;

  FormationList* myFormations = m_projectHandle->getFormations( snapshot, true );
  for ( const Formation* formation : *myFormations )
  {
    const string depthPropertyFormationName = depthDataGroup + "/" + formation->getMangledName();
    checkDataGroup( depthPropertyFormationName );
    DoubleVector depthVec = hdfReadManager.get3dCoordinatePropertyMatrix( {{ i, j}}, depthPropertyFormationName )[0];

    for ( int zi = 0; zi < depthVec.size() - 1; ++zi )
    {
      const double kf = getKfraction( depthVec[zi+1], depthVec[zi], z );
      if ( kf >= 0 && kf <= 1 )
      {
        const string propertyFormationDataGroup = propertyDataGroup + "/" + formation->getMangledName();
        checkDataGroup( propertyFormationDataGroup );
        DoubleVector propertyVec = hdfReadManager.get3dCoordinatePropertyMatrix( {{ i, j}}, propertyFormationDataGroup )[0];

        value = interpolate1d( propertyVec[zi+1], propertyVec[zi], kf );
        found = true;
        break;
      }
    }
    if ( found ) break;
  }

  hdfReadManager.closeSnapshotFile();
  delete myFormations;

  if ( !found )
  {
    throw RecordException( "Out of bound depth value: ", z );
  }
  return value;
}

double DataDriller::interpolate1d( const double u, const double l, const double k)
{
  return u + k * (l - u);
}

double DataDriller::getKfraction( const double u, const double  l, const double v)
{
  if ( std::fabs( l - u ) < 1e-10)
  {
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }
  return ( v - u )/(l - u);
}

} // namespace DataExtraction
