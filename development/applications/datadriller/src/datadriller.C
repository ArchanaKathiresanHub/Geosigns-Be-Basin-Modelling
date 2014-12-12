#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

// DataAccess API includes
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/Trap.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/ProjectHandle.h"

// EosPack
#include "EosPack.h"

// CBMGenerics
#include "ComponentManager.h"
#include "consts.h"
using namespace CBMGenerics;

#include "NumericFunctions.h"

#include "DataMiningProjectHandle.h"
#include "ElementPosition.h"
#include "Point.h"
#include "CauldronDomain.h"
#include "DomainProperty.h"
#include "DomainPropertyFactory.h"
#include "DomainPropertyCollection.h"

#include "errorhandling.h"

// STL
#include <algorithm>
#include <memory>

// STD C lib
#include <cassert>
#include <cstring>

using namespace std;

using namespace DataAccess;
using namespace Interface;
using namespace Numerics;
using namespace Mining;


const double StockTankPressure = 101325.0 * 1e-6;
const double StockTankTemperature = 15.0;

bool debug   = false;
bool verbose = false;

static double GetTrapPropertyValue( Mining::ProjectHandle* projectHandle, 
                                    const Interface::Property * property, const Interface::Snapshot * snapshot, const Interface::Reservoir * reservoir, double x, double y);
static double ComputeTrapPropertyValue (Mining::ProjectHandle* projectHandle, const Interface::Trap * trap, const Interface::Property * property, const Interface::Snapshot * snapshot, const Interface::Reservoir * reservoir, unsigned int i, unsigned int j);
static const Interface::GridMap * GetPropertyGridMap (Mining::ProjectHandle* projectHandle, const Interface::Property * property, const Interface::Snapshot * snapshot, const Interface::Reservoir * reservoir);
static bool performPVT (double masses[ComponentManager::NumberOfOutputSpecies], double temperature, double pressure,
                        double phaseMasses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], double phaseDensities[ComponentManager::NumberOfPhases], double phaseViscosities[ComponentManager::NumberOfPhases]);
static double Accumulate (double values[], int numberOfValues);
static double ComputeVolume (double * masses, double density, int numberOfSpecies);

static void showUsage ( const char* command, const char* message = 0 );

static int parseCmdLineArgs( int      argc
                           , char  ** argv
                           , string & inputProjectFileName
                           , string & outputProjectFileName
                           )
{
   inputProjectFileName  = "";
   outputProjectFileName = "";

   for ( int arg = 1; arg < argc; arg++ )
   {
      if ( strncmp( argv[arg], "-input", max<size_t>( 2, strlen( argv[arg] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc )
         {
            showUsage( argv[ 0 ], "Argument for '-input' is missing");
            return -1;
         }
         inputProjectFileName = argv[++arg];
      }
      else if ( strncmp( argv[arg], "-output", max<size_t>( 2, strlen( argv[arg] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc )
         {
            showUsage( argv[ 0 ], "Argument for '-output' is missing" );
            return -1;
         }
         outputProjectFileName = argv[++arg];
      }
      else if ( strncmp( argv[arg], "-debug",   max<size_t>( 2, strlen( argv[arg] ) ) ) == 0 ) { debug   = true; }
      else if ( strncmp( argv[arg], "-verbose", max<size_t>( 2, strlen( argv[arg] ) ) ) == 0 ) { verbose = true; }
      else if ( strncmp( argv[arg], "-help",    max<size_t>( 2, strlen( argv[arg] ) ) ) == 0 ) { showUsage( argv[ 0 ], "Standard usage."  ); return -1; }
      else if ( strncmp( argv[arg], "-?",       max<size_t>( 2, strlen( argv[arg] ) ) ) == 0 ) { showUsage( argv[ 0 ], "Standard usage."  ); return -1; }
      else if ( strncmp( argv[arg], "-usage",   max<size_t>( 2, strlen( argv[arg] ) ) ) == 0 ) { showUsage( argv[ 0 ], "Standard usage."  ); return -1; }
      else                                                                             { showUsage( argv[ 0 ], "Unknown argument" ); return -1; }

      if ( inputProjectFileName.empty()  ) { showUsage ( argv[ 0 ], "No project file specified"); return -1; }
      if ( outputProjectFileName.empty() ) { outputProjectFileName = inputProjectFileName;  }
   }
   return 0;
}

int main (int argc, char ** argv)
{
   string inputProjectFileName;
   string outputProjectFileName;

   if ( parseCmdLineArgs( argc, argv, inputProjectFileName, outputProjectFileName ) < 0 ) return -1;

   std::auto_ptr<Mining::DomainPropertyFactory> factory( new DataAccess::Mining::DomainPropertyFactory );
   Interface::ProjectHandle::UseFactory( factory.get() );

   std::auto_ptr<Mining::ProjectHandle> projectHandle( dynamic_cast<Mining::ProjectHandle*>( OpenCauldronProject( inputProjectFileName, "r" ) ) );

   projectHandle->startActivity( "datadriller", projectHandle->getLowResolutionOutputGrid ());
   projectHandle->initialise( true, false );

   projectHandle->setFormationLithologies( false, false );

   CauldronDomain domain( projectHandle.get() );

   DomainPropertyCollection * domainProperties = projectHandle->getDomainPropertyCollection();
   database::Table          * table            = projectHandle->getTable ("DataMiningIoTbl");

   const Interface::Grid * grid = projectHandle->getLowResolutionOutputGrid ();

   if ( table )
   {
      int recordIndex = 1;

      for ( database::Table::iterator tableIter = table->begin(); tableIter != table->end(); ++tableIter, ++recordIndex )
      {
         double                      value    = Interface::DefaultUndefinedScalarValue;
         database::Record          * record   = *tableIter;
         const Interface::Property * property = 0;

         try
         {
            double snapshotTime = database::getTime (record);
            if ( snapshotTime == Interface::DefaultUndefinedScalarValue ) throw RecordException( "Undefined Time value %:", snapshotTime );
            if ( snapshotTime < 0 )                                       throw RecordException( "Illegal snapshot time: %", snapshotTime );

            const Interface::Snapshot * snapshot = projectHandle->findSnapshot (snapshotTime);

            double x = database::getXCoord (record);
            double y = database::getYCoord (record);
            double z = database::getZCoord (record);

            if ( x == Interface::DefaultUndefinedScalarValue) throw RecordException ("Undefined XCoord value: %", x );
            if ( y == Interface::DefaultUndefinedScalarValue) throw RecordException ("Undefined YCoord value: %", y );

            const string & propertyName  = database::getPropertyName( record );
            const string & reservoirName = database::getReservoirName(record );
            const string & formationName = database::getFormationName(record );
            const string & surfaceName   = database::getSurfaceName(  record );

            property = projectHandle->findProperty( propertyName );
            if ( !property ) throw RecordException( "Unknown PropertyName value: %", propertyName );
            
            if ( reservoirName != "" )
            {
               const Interface::Reservoir * reservoir = projectHandle->findReservoir( reservoirName );
               if ( !reservoir ) throw RecordException( "Unknown ReservoirName value: %", reservoirName );

               value = GetTrapPropertyValue( projectHandle.get(), property, snapshot, reservoir, x, y );
            }
            else if ( z != Interface::DefaultUndefinedScalarValue || !surfaceName.empty() || !formationName.empty() )
            {
               domain.setSnapshot( snapshot );
               domainProperties->setSnapshot( snapshot );

               unsigned int i, j;
               if ( !grid->getGridPoint( x, y, i, j ) ) throw RecordException( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );

               ElementPosition element;
               
               if ( z != Interface::DefaultUndefinedScalarValue )
               {
                  if ( !domain.findLocation( x, y, z, element ) ) throw RecordException ("Illegal point coordinates:", x, y, z);
               }

               else if ( surfaceName != "" && formationName == "" )
               {
                  const Interface::Surface * surface = projectHandle->findSurface (surfaceName);
                  if ( !surface )                                       throw RecordException( "Unknown SurfaceName value: %", surfaceName );
                  if ( !domain.findLocation( x, y, surface, element ) ) throw RecordException( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );
               }

               else if ( formationName != "" && surfaceName == "" ) // z is also not defined - here we are having formation map property
               {
                  const Interface::Formation * formation = projectHandle->findFormation( formationName );
                  if ( !formation )                                       throw RecordException( "Unknown FormationName value: %", formationName );
                  if ( !domain.findLocation( x, y, formation, element ) ) throw RecordException ( "Illegal (XCoord, YCoord) pair: (%, %)", x, y );
               }
 
               DomainProperty * domainProperty = domainProperties->getDomainProperty( property );
               if ( domainProperty )
               {
                  domainProperty->initialise();
                  value = domainProperty->compute( element );
               }
            }
            else if ( formationName != "" && surfaceName != "" ) { throw RecordException( "Use of FormationName together with SurfaceName not yet implemented:" ); }
            else                                                 { throw RecordException( "Illegal specification" ); }
         }
         catch( const RecordException & recordException )
         {
            cerr << "Error in row " << recordIndex << " of DataMiningIoTbl: " << recordException.what () << endl;
         }

         database::setValue( record, value );
         if ( property ) { database::setPropertyUnit( record, property->getUnit() ); }
      }
   }

   projectHandle->saveToFile( outputProjectFileName );

   return 0;
}

double GetTrapPropertyValue( Mining::ProjectHandle      * projectHandle
                           , const Interface::Property  * property
                           , const Interface::Snapshot  * snapshot
                           , const Interface::Reservoir * reservoir
                           , double x
                           , double y
                           )
{
   const Interface::Property* trapIdProperty = projectHandle->findProperty ( "ResRockTrapId" );
   const Interface::GridMap * trapIdGridMap = GetPropertyGridMap (projectHandle, trapIdProperty, snapshot, reservoir);
   assert (trapIdGridMap);

   const Interface::Grid * grid = projectHandle->getHighResolutionOutputGrid ();
   assert (grid);

   unsigned int i, j;

   if (!grid->getGridPoint (x, y, i, j)) throw RecordException ("Illegal (XCoord, YCoord) pair: (%, %)", x, y);

   int trapId = (int) trapIdGridMap->getValue (i, j);

   if (trapId <= 0) throw RecordException ("No trap at (XCoord, YCoord) pair: (%, %)", x, y);

   if (debug) cerr << "Found trap id " << trapId << " at (" << i << ", " << j << ")" << endl;

   const Interface::Trap * trap = (const Interface::Trap *) projectHandle->findTrap (reservoir, snapshot, (unsigned int) trapId);
   if (!trap) throw RecordException ("Could not find trap");

   unsigned int trapI, trapJ;
   trap->getGridPosition (trapI, trapJ);

   return ComputeTrapPropertyValue (projectHandle, trap, property, snapshot, reservoir, trapI, trapJ);
}

double ComputeTrapPropertyValue (Mining::ProjectHandle* projectHandle, const Interface::Trap * trap, const Interface::Property * property, const Interface::Snapshot * snapshot, const Interface::Reservoir * reservoir, unsigned int i, unsigned int j)
{
   double value = Interface::DefaultUndefinedScalarValue;
   int comp;

   double masses[ComponentManager::NumberOfOutputSpecies];

   // reservoir condition phases
   double massesRC[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies];
   double densitiesRC[ComponentManager::NumberOfPhases];
   double viscositiesRC[ComponentManager::NumberOfPhases];

   // stock tank phases of reservoir condition phases
   double massesST[ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies];
   double densitiesST[ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases];
   double viscositiesST[ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases];

   for (comp = 0; comp < ComponentManager::NumberOfOutputSpecies; ++comp)
   {
      masses[comp] = trap->getMass ((Interface::ComponentId) (comp));
   }

   // perform PVT under reservoir conditions
   performPVT (masses, trap->getTemperature (), trap->getPressure (),
               massesRC, densitiesRC, viscositiesRC);

   // perform PVT's of reservoir condition phases under stock tank conditions
   performPVT (massesRC[ComponentManager::Vapour], StockTankTemperature, StockTankPressure,
               massesST[ComponentManager::Vapour], densitiesST[ComponentManager::Vapour], viscositiesST[ComponentManager::Vapour]);

   performPVT (massesRC[ComponentManager::Liquid], StockTankTemperature, StockTankPressure,
               massesST[ComponentManager::Liquid], densitiesST[ComponentManager::Liquid], viscositiesST[ComponentManager::Liquid]);

   bool stPhaseFound = false;
   bool rcPhaseFound = false;

   ComponentManager::PhaseId rcPhase;
   ComponentManager::PhaseId stPhase;

   const string & propertyName = property->getName ();

   if (propertyName.find ("FGIIP") != string::npos)
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Vapour;
   }
   else if (propertyName.find ("CIIP") != string::npos)
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Liquid;
   }
   else if (propertyName.find ("SGIIP") != string::npos)
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Vapour;
   }
   else if (propertyName.find ("STOIIP") != string::npos)
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Liquid;
   }
   else if (propertyName.find ("Vapour") != string::npos)
   {
      rcPhaseFound = true;
      rcPhase = ComponentManager::Vapour;
   }
   else if (propertyName.find ("Liquid") != string::npos)
   {
      rcPhaseFound = true;
      rcPhase = ComponentManager::Liquid;
   }

   // Volume, Density, Viscosity, and Mass properties for stock tank conditions
   if (stPhaseFound && propertyName.find ("Volume") != string::npos)
   {
      value = ComputeVolume (massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies);
   }
   else if (stPhaseFound && propertyName.find ("Density") != string::npos)
   {
      value = densitiesST[rcPhase][stPhase];
   }
   else if (stPhaseFound && propertyName.find ("Viscosity") != string::npos)
   {
      value = viscositiesST[rcPhase][stPhase];
   }
   else if (stPhaseFound && propertyName.find ("Mass") != string::npos)
   {
      value = Accumulate (massesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies);
      if (value < 1)
      {
         value = 0;
      }
   }
   // Volume, Density, Viscosity, and Mass properties for reservoir conditions
   else if (rcPhaseFound && propertyName.find ("Volume") != string::npos)
   {
      value = ComputeVolume (massesRC[rcPhase], densitiesRC[rcPhase], ComponentManager::NumberOfOutputSpecies);
   }
   else if (rcPhaseFound && propertyName.find ("Density") != string::npos)
   {
      value = densitiesRC[rcPhase];
   }
   else if (rcPhaseFound && propertyName.find ("Viscosity") != string::npos)
   {
      value = viscositiesRC[rcPhase];
   }
   else if (rcPhaseFound && propertyName.find ("Mass") != string::npos)
   {
      value = Accumulate (massesRC[rcPhase], ComponentManager::NumberOfOutputSpecies);
      if (value < 1)
      {
         value = 0;
      }
   }

   else if (propertyName == "CGR")
   {
      ComponentManager::PhaseId rcPhase;
      ComponentManager::PhaseId stPhase;

      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Liquid;

      double volumeCIIP = ComputeVolume (massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies);

      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Vapour;

      double volumeFGIIP = ComputeVolume (massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies);

      if (volumeFGIIP != 0)
      {
         value = volumeCIIP / volumeFGIIP;
      }
   }
   else if (propertyName == "GOR")
   {
      ComponentManager::PhaseId rcPhase;
      ComponentManager::PhaseId stPhase;

      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Liquid;

      double volumeSTOIIP = ComputeVolume (massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies);

      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Vapour;

      double volumeSGIIP = ComputeVolume (massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies);

      if (volumeSTOIIP != 0)
      {
         value = volumeSGIIP / volumeSTOIIP;
      }
   }
   else if (propertyName == "OilAPI")
   {
      if (densitiesST[ComponentManager::Liquid][ComponentManager::Liquid] != 0)
      {
         value = 141.5/(0.001*densitiesST[ComponentManager::Liquid][ComponentManager::Liquid]) -131.5;
      }
   }
   else if (propertyName == "CondensateAPI")
   {
      if (densitiesST[ComponentManager::Vapour][ComponentManager::Liquid] != 0)
      {
         value = 141.5/(0.001*densitiesST[ComponentManager::Vapour][ComponentManager::Liquid]) -131.5;
      }
   }
   // GasWetnessFGIIP and GasWetnessSGIIP
   else if (stPhaseFound && propertyName.find ("GasWetness") != string::npos)
   {
      pvtFlash::EosPack & eosPack = pvtFlash::EosPack::getInstance ();

      double moleC1 = massesST[rcPhase][stPhase][ComponentManager::C1] / eosPack.getMolWeight (ComponentManager::C1, 0);
      double moleC2 = massesST[rcPhase][stPhase][ComponentManager::C2] / eosPack.getMolWeight (ComponentManager::C2, 0);
      double moleC3 = massesST[rcPhase][stPhase][ComponentManager::C3] / eosPack.getMolWeight (ComponentManager::C3, 0);
      double moleC4 = massesST[rcPhase][stPhase][ComponentManager::C4] / eosPack.getMolWeight (ComponentManager::C4, 0);
      double moleC5 = massesST[rcPhase][stPhase][ComponentManager::C5] / eosPack.getMolWeight (ComponentManager::C5, 0);

      double moleC2_C5 = moleC2 + moleC3 + moleC4 + moleC5;

      if (moleC2_C5 != 0)
      {
         value = moleC1 / moleC2_C5;
      }
   }
   else if (propertyName == "CEPVapour")
   {
      value = database::getCEPGas (trap->getRecord ());
      if (value < 0)
      {
         value = Interface::DefaultUndefinedScalarValue;
      }
   }
   else if (propertyName == "CEPLiquid")
   {
      value = database::getCEPOil (trap->getRecord ());
      if (value < 0)
      {
         value = Interface::DefaultUndefinedScalarValue;
      }
   }
   else if (propertyName == "FracturePressure")
   {
      value = database::getFracturePressure (trap->getRecord ());
   }
   else if (propertyName == "ColumnHeightVapour")
   {
      value = database::getGOC (trap->getRecord ()) - database::getDepth (trap->getRecord ());
   }
   else if (propertyName == "ColumnHeightLiquid")
   {
      value = database::getOWC (trap->getRecord ()) - database::getGOC (trap->getRecord ());
   }
   else if (propertyName == "GOC")
   {
      value = database::getGOC (trap->getRecord ());
   }
   else if (propertyName == "OWC")
   {
      value = database::getOWC (trap->getRecord ());
   }
   else if (propertyName == "Depth")
   {
      value = database::getDepth (trap->getRecord ());
   }
   else if (propertyName == "SpillDepth")
   {
      value = database::getSpillDepth (trap->getRecord ());
   }
   else if (propertyName == "Pressure")
   {
      value = database::getPressure (trap->getRecord ());
   }
   else if (propertyName == "LithoStaticPressure" || propertyName == "HydroStaticPressure" || propertyName == "OverPressure")
   {
      CauldronDomain domain ( projectHandle );

      DomainPropertyCollection* domainProperties = projectHandle->getDomainPropertyCollection ();
      domain.setSnapshot (snapshot);
      domainProperties->setSnapshot (snapshot);

      double x, y;
      trap->getPosition (x, y);
      double z = trap->getDepth ();

      ElementPosition element;
      if (!domain.findLocation (x, y, z, element))
      {
         assert (false);
      }


      value = domainProperties->getDomainProperty (property)->compute (element);
   }
   else if (propertyName == "Temperature")
   {
      value = database::getTemperature (trap->getRecord ());
   }
   else if (propertyName == "Permeability")
   {
      value = database::getPermeability (trap->getRecord ());
   }
   else if (propertyName == "SealPermeability")
   {
      value = database::getSealPermeability (trap->getRecord ());
   }
   else if (propertyName == "Porosity")
   {
      const Interface::Property * reservoirProperty = projectHandle->findProperty ( "ResRockPorosity" );
      if (property)
      {
         const Interface::GridMap * reservoirPropertyGridMap = GetPropertyGridMap (projectHandle, reservoirProperty, snapshot, reservoir);
         if (reservoirPropertyGridMap) value = reservoirPropertyGridMap->getValue (i, j);
         reservoirPropertyGridMap->release ();
      }
   }
   else
   {
      throw RecordException ("PropertyName % not yet implemented:", propertyName);
   }

   return value;
}

const Interface::GridMap * GetPropertyGridMap (Mining::ProjectHandle* projectHandle, const Interface::Property * property, const Interface::Snapshot * snapshot, const Interface::Reservoir * reservoir)
{
   Interface::PropertyValueList * propertyValues = projectHandle->getPropertyValues (Interface::RESERVOIR, property, snapshot, reservoir, 0, 0, Interface::SURFACE);

   if (propertyValues->size () == 0)
   {
      throw RecordException ("Could not find value for property: %", property->getName ());
   }

   if (propertyValues->size () > 1)
   {
      throw RecordException ("Multiple values for property: %", property->getName ());
   }

   const Interface::GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

   delete propertyValues;
   return gridMap;
}

bool performPVT (double masses[ComponentManager::NumberOfOutputSpecies], double temperature, double pressure,
                 double phaseMasses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], double phaseDensities[ComponentManager::NumberOfPhases], double phaseViscosities[ComponentManager::NumberOfPhases])
{
   bool performedPVT = false;
   double massTotal = 0;

   int phase, comp;

   for (comp = 0; comp < ComponentManager::NumberOfOutputSpecies; ++comp)
   {
      massTotal += masses[comp];

      for (phase = 0; phase < ComponentManager::NumberOfPhases; ++phase)
      {
         phaseMasses[phase][comp] = 0;
      }
   }

   for (phase = 0; phase < ComponentManager::NumberOfPhases; ++phase)
   {
      phaseDensities[phase] = 0;
      phaseViscosities[phase] = 0;
   }

   if (massTotal > 100)
   {
      performedPVT = pvtFlash::EosPack::getInstance().computeWithLumping (temperature + C2K, pressure * MPa2Pa, masses, phaseMasses, phaseDensities, phaseViscosities);

   }

   return performedPVT;
}

double Accumulate (double values[], int numberOfValues)
{
   int i;

   double accumulatedValue = 0;

   for (i = 0; i < numberOfValues; ++i)
   {
      accumulatedValue += values[i];
   }

   return accumulatedValue;
}

double ComputeVolume (double * masses, double density, int numberOfSpecies)
{
   double value = Interface::DefaultUndefinedScalarValue;

   double mass = Accumulate (masses, numberOfSpecies);
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

void showUsage ( const char* command,
                 const char* message )
{

   std::cerr << std::endl;

   if ( message != 0 )
   {
      std::cerr << command << ": "  << message << std::endl;
   }

   std::cerr << "Usage: " << command << std::endl
             << "\t-input <cauldron-project-file>                     to specify input file" << std::endl
             << "\t[-output <cauldron-project-file>]                  to specify output file" << std::endl
             << "\t[-help]                                            to print this message." << std::endl
             << std::endl;

}


