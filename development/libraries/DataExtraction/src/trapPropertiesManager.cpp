//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "trapPropertiesManager.h"


// EosPack
#include "EosPack.h"

// utilities library
#include "errorhandling.h"
#include "ConstantsMathematics.h"

#include <cassert>

using Utilities::Maths::CelciusToKelvin;
using Utilities::Maths::MegaPaToPa;

namespace DataExtraction
{

static const double StockTankPressure = 1.01325e-1;
static const double StockTankTemperature = 15.0;

TrapPropertiesManager::TrapPropertiesManager( Mining::ProjectHandle* projectHandle,
                                              DerivedProperties::DerivedPropertyManager& propertyManager ) :
  m_projectHandle( projectHandle ),
  m_propertyManager( propertyManager )
{
}

double TrapPropertiesManager::getTrapPropertyValue( const Interface::Property  * property,
                                                    const Interface::Snapshot  * snapshot,
                                                    const Interface::Reservoir * reservoir,
                                                    double x,
                                                    double y )
{
  const Interface::Grid * grid = m_projectHandle->getActivityOutputGrid();

  assert( grid );
  assert( grid == m_projectHandle->getHighResolutionOutputGrid() ); // expecting that this activity always dealing with high resolution grid

  const Interface::Property * trapIdProperty = m_projectHandle->findProperty( "ResRockTrapId" );
  AbstractDerivedProperties::ReservoirPropertyPtr trapIdGridMap = getReservoirPropertyGridMap( trapIdProperty, snapshot, reservoir );
  assert (( "Unable to find ResRockTrapId map", trapIdGridMap ));

  unsigned int i, j;

  if( !grid->getGridPoint( x, y, i, j ) ) throw RecordException( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );

  int trapId = static_cast<int>( trapIdGridMap->get( i, j ));

  if ( trapId <= 0 ) throw RecordException( "No trap at (XCoord, YCoord) pair: (%, %)", x, y );

  const Interface::Trap * trap = m_projectHandle->findTrap( reservoir, snapshot, (unsigned int)trapId );
  if ( !trap ) throw RecordException( "Could not find trap" );

  trap->getGridPosition( i, j );

  return computeTrapPropertyValue( trap, property, snapshot, reservoir, i, j );
}

double TrapPropertiesManager::computeTrapPropertyValue( const Interface::Trap      * trap,
                                                        const Interface::Property  * property,
                                                        const Interface::Snapshot  * snapshot,
                                                        const Interface::Reservoir * reservoir,
                                                        unsigned int i,
                                                        unsigned int j )
{
  double value = Interface::DefaultUndefinedScalarValue;

  double masses[ComponentManager::NUMBER_OF_SPECIES];

  // reservoir condition phases
  double massesRC[     ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES];
  double densitiesRC[  ComponentManager::NUMBER_OF_PHASES];
  double viscositiesRC[ComponentManager::NUMBER_OF_PHASES];

  // stock tank phases of reservoir condition phases
  double massesST[     ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES];
  double densitiesST[  ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES];
  double viscositiesST[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES];

  for ( int comp = 0; comp < ComponentManager::NUMBER_OF_SPECIES; ++comp)
  {
    masses[comp] = trap->getMass( ComponentManager::SpeciesNamesId( comp ) );
  }

  // perform PVT under reservoir conditions
  bool pvtRC = performPVT( masses, trap->getTemperature(), trap->getPressure(), massesRC, densitiesRC, viscositiesRC );

  double phaseMassesRC[2] = { 0.0, 0.0 };
  for ( int comp = 0; pvtRC && comp < ComponentManager::NUMBER_OF_SPECIES; ++comp )
  {
    phaseMassesRC[ComponentManager::VAPOUR] += massesRC[ComponentManager::VAPOUR][comp];
    phaseMassesRC[ComponentManager::LIQUID] += massesRC[ComponentManager::LIQUID][comp];
  }

  // perform PVT's of reservoir condition phases under stock tank conditions
  bool pvtRCVapour = performPVT( massesRC[ComponentManager::VAPOUR], StockTankTemperature, StockTankPressure,
      massesST[ComponentManager::VAPOUR], densitiesST[ComponentManager::VAPOUR], viscositiesST[ComponentManager::VAPOUR] );

  bool pvtRCLiquid = performPVT( massesRC[ComponentManager::LIQUID], StockTankTemperature, StockTankPressure,
      massesST[ComponentManager::LIQUID], densitiesST[ComponentManager::LIQUID], viscositiesST[ComponentManager::LIQUID] );

  bool stPhaseFound = false;
  bool rcPhaseFound = false;

  ComponentManager::PhaseId rcPhase;
  ComponentManager::PhaseId stPhase;

  const string & propertyName = property->getName();

  if ( propertyName.find( "FGIIP" ) != string::npos )
  {
    stPhaseFound = true;
    rcPhase = ComponentManager::VAPOUR;
    stPhase = ComponentManager::VAPOUR;
  }
  else if (propertyName.find( "CIIP" ) != string::npos )
  {
    stPhaseFound = true;
    rcPhase = ComponentManager::VAPOUR;
    stPhase = ComponentManager::LIQUID;
  }
  else if (propertyName.find( "SGIIP" ) != string::npos )
  {
    stPhaseFound = true;
    rcPhase = ComponentManager::LIQUID;
    stPhase = ComponentManager::VAPOUR;
  }
  else if ( propertyName.find( "STOIIP" ) != string::npos )
  {
    stPhaseFound = true;
    rcPhase = ComponentManager::LIQUID;
    stPhase = ComponentManager::LIQUID;
  }
  else if ( propertyName.find( "Vapour" ) != string::npos )
  {
    rcPhaseFound = true;
    rcPhase = ComponentManager::VAPOUR;
  }
  else if ( propertyName.find( "Liquid" ) != string::npos )
  {
    rcPhaseFound = true;
    rcPhase = ComponentManager::LIQUID;
  }

  // Volume, Density, Viscosity, and Mass properties for stock tank conditions
  if ( stPhaseFound && propertyName.find( "Volume" ) != string::npos )
  {
    value = computeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );
  }
  else if ( propertyName.find( "Density" ) != string::npos )
  {
    if ( stPhaseFound )
    {
      if ( (rcPhase == ComponentManager::VAPOUR && pvtRCVapour) || (rcPhase == ComponentManager::LIQUID && pvtRCLiquid) )
      {
        value = densitiesST[rcPhase][stPhase];
      }
    }
    else if ( pvtRC && phaseMassesRC[rcPhase] > 0.0 ) { value = densitiesRC[rcPhase]; }
  }
  else if ( propertyName.find( "Viscosity" ) != string::npos )
  {
    if ( stPhaseFound )
    {
      if ( (rcPhase == ComponentManager::VAPOUR && pvtRCVapour) || (rcPhase == ComponentManager::LIQUID && pvtRCLiquid) )
      {
        value = viscositiesST[rcPhase][stPhase];
      }
    }
    else if ( pvtRC && phaseMassesRC[rcPhase] > 0.0 ) { value = viscositiesRC[rcPhase]; }
  }
  else if ( stPhaseFound && propertyName.find( "Mass" ) != string::npos )
  {
    value = accumulate( massesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );
    if ( value < 1 )
    {
      value = 0;
    }
  }
  // Volume, Density, Viscosity, and Mass properties for reservoir conditions
  else if ( rcPhaseFound && propertyName.find( "Volume" ) != string::npos )
  {
    value = computeVolume( massesRC[rcPhase], densitiesRC[rcPhase], ComponentManager::NUMBER_OF_SPECIES );
  }
  else if ( rcPhaseFound && propertyName.find( "Mass" ) != string::npos )
  {
    value = accumulate( massesRC[rcPhase], ComponentManager::NUMBER_OF_SPECIES );
    if ( value < 1 )
    {
      value = 0;
    }
  }
  else if ( propertyName == "CGR" )
  {
    ComponentManager::PhaseId rcPhase;
    ComponentManager::PhaseId stPhase;

    rcPhase = ComponentManager::VAPOUR;
    stPhase = ComponentManager::LIQUID;

    double volumeCIIP = computeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

    rcPhase = ComponentManager::VAPOUR;
    stPhase = ComponentManager::VAPOUR;

    double volumeFGIIP = computeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

    if (volumeFGIIP != 0)
    {
      value = volumeCIIP / volumeFGIIP;
    }
  }
  else if ( propertyName == "GOR" )
  {
    ComponentManager::PhaseId rcPhase;
    ComponentManager::PhaseId stPhase;

    rcPhase = ComponentManager::LIQUID;
    stPhase = ComponentManager::LIQUID;

    double volumeSTOIIP = computeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

    rcPhase = ComponentManager::LIQUID;
    stPhase = ComponentManager::VAPOUR;

    double volumeSGIIP = computeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

    if ( volumeSTOIIP != 0 )
    {
      value = volumeSGIIP / volumeSTOIIP;
    }
  }
  else if ( propertyName == "OilAPI" )
  {
    if ( densitiesST[ComponentManager::LIQUID][ComponentManager::LIQUID] != 0 )
    {
      value = 141.5/(0.001*densitiesST[ComponentManager::LIQUID][ComponentManager::LIQUID]) - 131.5;
    }
  }
  else if ( propertyName == "CondensateAPI" )
  {
    if ( densitiesST[ComponentManager::VAPOUR][ComponentManager::LIQUID] != 0 )
    {
      value = 141.5/(0.001*densitiesST[ComponentManager::VAPOUR][ComponentManager::LIQUID]) - 131.5;
    }
  }
  // GasWetnessFGIIP and GasWetnessSGIIP
  else if ( stPhaseFound && propertyName.find ( "GasWetness" ) != string::npos )
  {
    pvtFlash::EosPack & eosPack = pvtFlash::EosPack::getInstance ();

    double moleC1 = massesST[rcPhase][stPhase][ComponentManager::C1] / eosPack.getMolWeight( ComponentManager::C1, 0 );
    double moleC2 = massesST[rcPhase][stPhase][ComponentManager::C2] / eosPack.getMolWeight( ComponentManager::C2, 0 );
    double moleC3 = massesST[rcPhase][stPhase][ComponentManager::C3] / eosPack.getMolWeight( ComponentManager::C3, 0 );
    double moleC4 = massesST[rcPhase][stPhase][ComponentManager::C4] / eosPack.getMolWeight( ComponentManager::C4, 0 );
    double moleC5 = massesST[rcPhase][stPhase][ComponentManager::C5] / eosPack.getMolWeight( ComponentManager::C5, 0 );

    double moleC2_C5 = moleC2 + moleC3 + moleC4 + moleC5;

    if (moleC2_C5 != 0)
    {
      value = moleC1 / moleC2_C5;
    }
  }
  else if ( propertyName == "CEPVapour" )
  {
    value = database::getCEPGas( trap->getRecord() );
    if ( value < 0 )
    {
      value = Interface::DefaultUndefinedScalarValue;
    }
  }
  else if ( propertyName == "CEPLiquid" )
  {
    value = database::getCEPOil( trap->getRecord() );
    if ( value < 0 )
    {
      value = Interface::DefaultUndefinedScalarValue;
    }
  }
  else if ( propertyName == "FracturePressure"   ) { value = database::getFracturePressure( trap->getRecord() ); }
  else if ( propertyName == "ColumnHeightVapour" ) { value = database::getGOC(              trap->getRecord() ) - database::getDepth( trap->getRecord() ); }
  else if ( propertyName == "ColumnHeightLiquid" ) { value = database::getOWC(              trap->getRecord() ) - database::getGOC( trap->getRecord() ); }
  else if ( propertyName == "GOC"                ) { value = database::getGOC(              trap->getRecord() ); }
  else if ( propertyName == "OWC"                ) { value = database::getOWC(              trap->getRecord() ); }
  else if ( propertyName == "Depth"              ) { value = database::getDepth(            trap->getRecord() ); }
  else if ( propertyName == "SpillDepth"         ) { value = database::getSpillDepth(       trap->getRecord() ); }
  else if ( propertyName == "Pressure"           ) { value = database::getPressure(         trap->getRecord() ); }
  else if ( propertyName == "Temperature"        ) { value = database::getTemperature(      trap->getRecord() ); }
  else if ( propertyName == "Permeability"       ) { value = database::getPermeability(     trap->getRecord() ); }
  else if ( propertyName == "SealPermeability"   ) { value = database::getSealPermeability( trap->getRecord() ); }

  else if ( propertyName == "LithoStaticPressure" || propertyName == "HydroStaticPressure" || propertyName == "OverPressure" )
  {
    CauldronDomain           & domain           = m_projectHandle->getCauldronDomain();
    DomainPropertyCollection * domainProperties = m_projectHandle->getDomainPropertyCollection();

    domain.setSnapshot( snapshot, m_propertyManager );
    domainProperties->setSnapshot( snapshot );

    double x, y;
    trap->getPosition( x, y );
    double z = trap->getDepth();

    ElementPosition element;
    if ( !domain.findLocation( x, y, z, element ) )
    {
      assert( false );
    }
    value = domainProperties->getDomainProperty( property, m_propertyManager )->compute( element );
  }
  else if ( propertyName == "Porosity" )
  {
    const Interface::Property * reservoirProperty = m_projectHandle->findProperty( "ResRockPorosity" );
    if ( property )
    {
      AbstractDerivedProperties::ReservoirPropertyPtr reservoirPropertyGridMap =
          getReservoirPropertyGridMap( reservoirProperty, snapshot, reservoir );
      // const Interface::GridMap * reservoirPropertyGridMap = GetPropertyGridMap( projectHandle, reservoirProperty, snapshot, reservoir );

      if ( reservoirPropertyGridMap ) {
        value = reservoirPropertyGridMap->get ( i, j );
      }

      // reservoirPropertyGridMap->release ();
    }
  }
  else if ( propertyName.rfind( "TrappedAmount" ) != std::string::npos )
  {
    bool found = false;
    for ( int i = 0; i < ComponentManager::NUMBER_OF_SPECIES && !found; ++i )
    {
      if ( !propertyName.compare( 0, propertyName.length() - 13, ComponentManager::getInstance().getSpeciesName( i ) ) )
      {
        value = masses[i];
        found = true;
      }
    }
    if ( !found ) { throw RecordException ("PropertyName % not yet implemented:", propertyName); }
  }
  else
  {
    throw RecordException ("PropertyName % not yet implemented:", propertyName);
  }

  return value;
}

AbstractDerivedProperties::ReservoirPropertyPtr
TrapPropertiesManager::getReservoirPropertyGridMap( const Interface::Property* property,
                                                     const Interface::Snapshot* snapshot,
                                                     const Interface::Reservoir* reservoir )
{
  AbstractDerivedProperties::ReservoirPropertyPtr result;

  if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY )
  {
    result = m_propertyManager.getReservoirProperty ( property, snapshot, reservoir );
  }

  if ( result == 0 )
  {
    throw RecordException( "Could not find value for property: %", property->getName() );
  }

  return result;
}

const Interface::GridMap* TrapPropertiesManager::getPropertyGridMap( const Interface::Property* property,
                                                                     const Interface::Snapshot* snapshot,
                                                                     const Interface::Reservoir* reservoir )
{
  Interface::PropertyValueList * propertyValues = m_projectHandle->getPropertyValues( Interface::RESERVOIR, property, snapshot, reservoir, 0, 0, Interface::SURFACE );

  if ( propertyValues->size() == 0 ) { throw RecordException( "Could not find value for property: %", property->getName() ); }
  if ( propertyValues->size()  > 1 ) { throw RecordException( "Multiple values for property: %",      property->getName() ); }

  const Interface::GridMap * gridMap = (*propertyValues)[0]->getGridMap();

  delete propertyValues;
  return gridMap;
}

bool TrapPropertiesManager::performPVT( double masses[ComponentManager::NUMBER_OF_SPECIES],
double temperature,
double pressure,
double phaseMasses[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES],
double phaseDensities[ComponentManager::NUMBER_OF_PHASES],
double phaseViscosities[ComponentManager::NUMBER_OF_PHASES] )
{
  bool performedPVT = false;
  double massTotal = 0;

  for ( int comp = 0; comp < ComponentManager::NUMBER_OF_SPECIES; ++comp )
  {
    massTotal += masses[comp];

    for ( int phase = 0; phase < ComponentManager::NUMBER_OF_PHASES; ++phase )
    {
      phaseMasses[phase][comp] = 0;
    }
  }

  for ( int phase = 0; phase < ComponentManager::NUMBER_OF_PHASES; ++phase )
  {
    phaseDensities[phase] = 0;
    phaseViscosities[phase] = 0;
  }

  if ( massTotal > 100 )
  {
    performedPVT = pvtFlash::EosPack::getInstance().computeWithLumping( temperature + CelciusToKelvin, pressure * MegaPaToPa, masses, phaseMasses, phaseDensities, phaseViscosities );
  }

  return performedPVT;
}

double TrapPropertiesManager::accumulate( double values[], int numberOfValues )
{
  double accumulatedValue = 0;

  for ( int i = 0; i < numberOfValues; ++i )
  {
    accumulatedValue += values[i];
  }

  return accumulatedValue;
}

double TrapPropertiesManager::computeVolume (double * masses, double density, int numberOfSpecies)
{
  double value = Interface::DefaultUndefinedScalarValue;

  double mass = accumulate (masses, numberOfSpecies);
  if (mass < 1)
  {
    value = 0;
  }
  else
  {
    if (density == 0) throw RecordException ("zero density computed with non-zero mass");

    value = mass / density;
  }
  return value;
}

} // namespace DataExtraction
