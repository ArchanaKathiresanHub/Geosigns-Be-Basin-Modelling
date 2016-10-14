#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>

#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <cstdlib>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

using namespace std;

// DataAccess API includes
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/ProjectHandle.h"
#include "Interface/SimulationDetails.h"

#include "DerivedPropertyManager.h"

#include "NumericFunctions.h"

#include "DataMiningProjectHandle.h"
#include "ElementPosition.h"
#include "Point.h"
#include "PropertyInterpolator2D.h"
#include "PropertyInterpolator3D.h"
#include "CauldronDomain.h"
#include "InterpolatedPropertyValues.h"
#include "DomainProperty.h"
#include "DomainPropertyFactory.h"
#include "DomainPropertyCollection.h"

#include "DomainSurfaceProperty.h"
#include "DomainFormationProperty.h"
#include "DomainReservoirProperty.h"

#include "PieceWiseInterpolator1D.h"

#include "DeviatedWell.h"
#include "VerticalWell.h"
#include "DataMiner.h"

#include "PorosityCalculator.h"
#include "PermeabilityCalculator.h"

#include "DeviatedWellData.h"

#include "WellWriter.h"
#include "WellWriterFactory.h"

#include "array.h"

#include <string>
#include <vector>
using namespace std;

using namespace DataAccess;
using namespace Interface;
using namespace Numerics;
using namespace Mining;

const bool DefaultInterFormationBoundary = true;
const bool DefaultInterPlanarElementBoundary = false;
const bool DefaultInterVerticalElementBoundary = false;

static void showUsage ( const char* command,
                        const char* message = 0);


CauldronWell* getCauldronWell ( const DeviatedWellData& data,
                                const CauldronDomain& domain );

void addAlternativePropertyNames ( Mining::ProjectHandle* projectHandle );

void addDefaultProperties ( const Mining::ProjectHandle* projectHandle,
                            DataMiner::PropertySet& properties );


int main (int argc, char ** argv) {

   string projectFileName;
   string wellFileName;
   string outputFileFormat;
   string outputFileName;
   string userDefinedPropertyNames;

   double snapshotTime = 0;
   bool userDefinedProperties = false;
   bool verbose = false;
   bool debug = false;
   bool listAllProperties = false;
   bool listSavedProperties = false;

   bool interFormationBoundary = DefaultInterFormationBoundary;
   bool interPlanarElementBoundary = DefaultInterPlanarElementBoundary;
   bool interVerticalElementBoundary = DefaultInterVerticalElementBoundary;

   int arg;
   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-project", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-project' is missing");
            return -1;
         }
         projectFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-well", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-well' is missing");
            return -1;
         }
         wellFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-output", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-output' is missing");
            return -1;
         }

         outputFileFormat = argv[++arg];
      }
      else if (strncmp (argv[arg], "-save", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-save' is missing");
            return -1;
         }

         outputFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-properties", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-properties' is missing");
            return -1;
         }

         userDefinedPropertyNames = argv[++arg];
         userDefinedProperties = true;
      }
      else if (strncmp (argv[arg], "-snapshot", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-snapshot' is missing");
            return -1;
         }
         snapshotTime = atof (argv[++arg]);
      }
      else if (strncmp (argv[arg], "-debug", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if (strncmp (argv[arg], "-interform", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
        interFormationBoundary = true;
      }
      else if (strncmp (argv[arg], "-nointerform", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
        interFormationBoundary = false;
      }
      else if (strncmp (argv[arg], "-interplaneelem", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
        interPlanarElementBoundary = true;
      }
      else if (strncmp (argv[arg], "-nointerplaneelem", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
        interPlanarElementBoundary = false;
      }
      else if (strncmp (argv[arg], "-interverticalelem", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
        interVerticalElementBoundary = true;
      }
      else if (strncmp (argv[arg], "-nointerverticalelem", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
        interVerticalElementBoundary = false;
      }
      else if ((strncmp (argv[arg], "-help", NumericFunctions::Maximum<size_t> (2, strlen(argv[arg]))) == 0) ||
		       (strncmp(argv[arg], "-?",     NumericFunctions::Maximum<size_t> (2, strlen(argv[arg]))) == 0) ||
		       (strncmp(argv[arg], "-usage", NumericFunctions::Maximum<size_t> (2, strlen(argv[arg]))) == 0))
      {
         showUsage ( argv[ 0 ], " Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-verbose", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         verbose = true;
      }
      else if (strncmp (argv[arg], "-listall", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         listAllProperties = true;
      }
      else if (strncmp (argv[arg], "-listsaved", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         listSavedProperties = true;
      }
      else
      {
         showUsage ( argv[ 0 ]);
         return -1;
      }
   }

   if (projectFileName == "")
   {
      showUsage ( argv[ 0 ], "No project file specified");
      return -1;
   }

   if (wellFileName == "")
   {
      showUsage ( argv[ 0 ], "No well file specified");
      return -1;
   }

   if ( outputFileName == "" ) {
      showUsage ( argv[ 0 ], "No output file specified");      
      return -1;
   } else if ( outputFileFormat == "" ) {
      size_t dotPosition = outputFileName.rfind ( "." );

      if ( dotPosition != std::string::npos ) {
         outputFileFormat = outputFileName.substr ( dotPosition + 1 );
      }

   }

   if ( outputFileFormat == "" ) {
      std::cerr << " Warning: output format has not been defined. Using default " 
                << '\'' << WellWriterFactory::getInstance ().getDefaultWriterIdentifier () << '\'' << std::endl;
      outputFileFormat = WellWriterFactory::getInstance ().getDefaultWriterIdentifier ();
   } else if ( not WellWriterFactory::getInstance ().outputFormatIsDefined ( outputFileFormat )) {
      std::stringstream buffer;

      buffer << " Error: output format " << outputFileFormat << " not defined." << std::endl;

      showUsage ( argv[ 0 ], buffer.str ().c_str ());
      return -1;
   }

   // writer should not be null after this call because of the above check.
   // To see if the output-file-format was defined in the well-writer-factory.
   WellWriter* writer = WellWriterFactory::getInstance ().allocate ( outputFileFormat );

   // Add file-extension if correct value is not already there.
   if ( outputFileName.find ( '.' + writer->getExtension ()) == std::string::npos ) {
      outputFileName = outputFileName + '.' + writer->getExtension ();
   }

   DeviatedWellData wellData ( wellFileName );

   if ( verbose ) {
      std::cerr << "Read input well description file." << std::endl;
   }

   Mining::DomainPropertyFactory* factory = new DataAccess::Mining::DomainPropertyFactory;

   Mining::ProjectHandle* projectHandle = (Mining::ProjectHandle*)(OpenCauldronProject (projectFileName, "r", factory));
   DerivedProperties::DerivedPropertyManager propertyManager ( projectHandle );

   projectHandle->startActivity ( "deviatedwell", projectHandle->getLowResolutionOutputGrid ());
   projectHandle->initialise ( true, false );

   if ( not projectHandle->setFormationLithologies ( false, true )) {
      std::cerr << " Cannot set lithologies." << std::endl;
   }

   bool coupledCalculation = false;

   const Interface::SimulationDetails* simulationDetails = projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

   if ( simulationDetails != 0 ) {
      coupledCalculation = simulationDetails->getSimulatorMode () == "Overpressure" or
                           simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" or
                           simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" or
                           simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" or
                           simulationDetails->getSimulatorMode () == "CoupledDarcy";
   } else {
      // If this table is not present the assume that the last
      // fastcauldron mode was not pressure mode.
      // This table may not be present because we are running c2e on an old 
      // project, before this table was added.
      coupledCalculation = false;
   }

   projectHandle->initialiseLayerThicknessHistory ( coupledCalculation );

   if ( listAllProperties ) {
      projectHandle->listProperties ( std::cout );
      return -1;
   }

   if ( listSavedProperties ) {
      projectHandle->listSavedProperties ( std::cout );
      return -1;
   }

   CauldronDomain& domain = projectHandle->getCauldronDomain ();
   const Interface::Snapshot* snapshot = projectHandle->findSnapshot ( snapshotTime );

   projectHandle->switchLithologies ( snapshot->getTime ());

   DomainPropertyCollection* domainProperties = projectHandle->getDomainPropertyCollection ();

   domain.setSnapshot ( snapshot, propertyManager );
   domainProperties->setSnapshot ( snapshot );

   DataMiner miner ( projectHandle, propertyManager );

   ElementPositionSequence positions;
   DataMiner::ResultValues results;
   DataMiner::PropertySet properties;

   PieceWiseInterpolator1D interp;
   ElementPosition element;

   addAlternativePropertyNames ( projectHandle );

   if ( userDefinedProperties ) {
      // userDefinedPropertyNames;

      const Interface::Property* property;
      std::string propertyName;
      std::string userDefinedPropertySubString = userDefinedPropertyNames;
      size_t commaPosition;

      // Extract property-names from comma-separated list.
      do {
         commaPosition = userDefinedPropertySubString.find ( ',' );
         propertyName = userDefinedPropertySubString.substr ( 0, commaPosition );

         property = projectHandle->findProperty ( projectHandle->getPropertyNameFromAlternative ( propertyName ));

         if ( property != 0 ) {
            properties.push_back ( property );
         } else if ( propertyName != "" ) {
            std::cerr << " Error: unable to find property with name: >" << propertyName << "<" << std::endl;
         }

         userDefinedPropertySubString = userDefinedPropertySubString.substr ( commaPosition + 1 );

      } while ( commaPosition != std::string::npos );

   } else {
      addDefaultProperties ( projectHandle, properties );
   }

   if ( verbose or debug ) {
      size_t i;

      for ( i = 0; i < properties.size (); ++i ) {
         std::cerr << " Property " << std::setw ( 2 ) << i + 1 << ": " << properties [ i ]->getName () << std::endl;
      }

   }

   ElementPositionSequence elements;

   CauldronWell* well = getCauldronWell ( wellData, domain );

   domain.findWellPath ( *well, elements, interFormationBoundary, interPlanarElementBoundary, interVerticalElementBoundary );

   if ( verbose ) {
      std::cerr << "Found all elements along well path." << std::endl;
   }

   if ( debug ) {
      std::cout << std::endl << wellData.image () << std::endl << std::endl;
   }

   if ( debug ) {
      size_t i;

      for ( i = 0; i < elements.size (); ++i ) {
         cout << elements [ i ].image () << endl;
      }

   }

   miner.setProperties ( properties );
   miner.compute ( elements, properties, results );

   if ( verbose ) {
      std::cerr << "Computed all properties along well path." << std::endl;
   }

   if ( writer != 0 ) {
      writer->write ( outputFileName, elements, *well, results, properties, wellData.getUnit ());
      delete writer;
   }

   if ( verbose ) {
      std::cerr << "Wrote data to file." << std::endl;
   }

   WellWriterFactory::finalise ();
   delete factory;
   return 0;
}


CauldronWell* getDeviatedWell ( const DeviatedWellData& data,
                                const CauldronDomain&   domain ) {

   DeviatedWell* well = new DeviatedWell ( data.getName ());

   const WellTrajectoryLocationArray& trajectory = data.getTrajectory ();
   size_t i;
   double depthAlongHole = 0.0;
   Numerics::PieceWiseInterpolator1D::InterpolationKind interpolationKind = data.getInterpolationKind ();

   for ( i = 0; i < trajectory.size (); ++i ) {

      if ( i > 0 ) {
         depthAlongHole += separationDistance ( trajectory [ i - 1 ].position (), trajectory [ i ].position ());
      }

      if ( trajectory [ i ].depthAlongHole () == DataAccess::Interface::DefaultUndefinedMapValue ) {
         well->addLocation ( trajectory [ i ].position (), depthAlongHole );
      } else {
         well->addLocation ( trajectory [ i ].position (), trajectory [ i ].depthAlongHole ());
      }

   }

   well->freeze ( interpolationKind );

   return well;
}

CauldronWell* getVerticalWell ( const DeviatedWellData& data,
                                const CauldronDomain&   domain ) {

   VerticalWell* well = new VerticalWell ( data.getName ());

   ElementPosition topElement;
   ElementPosition bottomElement;

   Numerics::Point top = data.getTrajectory ()[ 0 ].position ();
   top ( 2 ) = 0.0;

   domain.getTopSurface ( top ( 0 ), top ( 1 ), topElement, false );
   domain.getBottomSurface ( top ( 0 ), top ( 1 ), bottomElement, false );

   well->setTop ( topElement.getActualPoint ());
   well->setLength ( bottomElement.getActualPoint ()( 2 ) - topElement.getActualPoint ()( 2 ));

   return well;
}

CauldronWell* getCauldronWell ( const DeviatedWellData& data,
                                const CauldronDomain&   domain ) {

   CauldronWell* well;

   if ( data.getTrajectory ().size () == 1 ) {
      well = getVerticalWell ( data, domain );
   } else {
      well = getDeviatedWell ( data, domain );
   }

   well->setNullValue ( data.getNullValue ());
   well->setElevation ( data.getElevation ());
   well->setWaterDepth ( data.getWaterDepth ());
   well->setKellyBushingDepth ( data.getKellyBushingDepth ());
   well->setSamplingResolution ( data.getDeltaS ());

   return well;
}


void showUsage ( const char* command,
                 const char* message ) {

   static std::string text [ 2 ] = { "NOT TO ADD", "TO ADD"};

   std::cerr << std::endl;

   if ( message != 0 ) {
      std::cerr << std::endl;
      std::cerr << command << ": "  << message << std::endl;
      std::cerr << std::endl;
   }

   std::cerr << "Usage: " << command << std::endl
             << std::endl 
             << "     -project <cauldron-project-file>" << std::endl
             << "           The Cauldron project from which results are to be extracted." << std::endl
             << std::endl
             << "     -help" << std::endl
             << "           Print this message." << std::endl
             << std::endl 
             << "     -output <type>" << std::endl
             << "           Output file format, default: " << WellWriterFactory::getInstance ().getDefaultWriterIdentifier () << "." << std::endl
             << "           Choices for <type> are: csv, matlab, cout." << std::endl
             << std::endl 
             << "     -save <output-file-name>" << std::endl
             << "           Set the output file name." << std::endl
             << std::endl 
             << "     -well <well-def-file>" << std::endl
             << "           Input well definition file." << std::endl
             << std::endl 
             << "     -interform" << std::endl
             << "     -nointerform" << std::endl
             << "           Control whether or not inter-formation elements are to be added." << std::endl
             << "           Default is " << text [ DefaultInterFormationBoundary ] << " inter-formation elements." << std::endl
             << std::endl 
             << "     -interplaneelem" << std::endl
             << "     -nointerplaneelem" << std::endl
             << "           Control whether or not inter-planar elements are to be added." << std::endl
             << "           Default is " << text [ DefaultInterPlanarElementBoundary ] << " inter-planar elements." << std::endl
             << std::endl 
             << "     -interverticalelem" << std::endl
             << "     -nointerverticalelem" << std::endl
             << "           Control whether or not inter-vertical elements are to be added." << std::endl
             << "           Default is " << text [ DefaultInterVerticalElementBoundary ] << " inter-vertical elements." << std::endl
             << std::endl 
             << "     -snapshot <age>" << std::endl
             << "           The snapshot-age to be used for the extraction." << std::endl
             << "           Recommended for vertical wells only."  << std::endl
             << std::endl 
             << "     -properties <comma-separated list of properties>"  << std::endl
             << "           Over-rides the default set of properties, sample position information is still output."
             << std::endl 
             << std::endl;

}

void addAlternativePropertyNames ( Mining::ProjectHandle* projectHandle ) {

   projectHandle->addAlternativeName ( "Temperature", "temperature" );
   projectHandle->addAlternativeName ( "Temperature", "temp" );
   projectHandle->addAlternativeName ( "Temperature", "Temp" );

   projectHandle->addAlternativeName ( "OverPressure", "Overpressure" );
   projectHandle->addAlternativeName ( "OverPressure", "overpressure" );

   projectHandle->addAlternativeName ( "Pressure", "pressure" );
   projectHandle->addAlternativeName ( "Pressure", "porepressure" );
   projectHandle->addAlternativeName ( "Pressure", "PorePressure" );

   projectHandle->addAlternativeName ( "HydroStaticPressure", "HydrostaticPressure" );
   projectHandle->addAlternativeName ( "HydroStaticPressure", "HydroStaticpressure" );
   projectHandle->addAlternativeName ( "HydroStaticPressure", "Hydrostaticpressure" );
   projectHandle->addAlternativeName ( "HydroStaticPressure", "hydrostaticpressure" );

   projectHandle->addAlternativeName ( "LithoStaticPressure", "LithostaticPressure" );
   projectHandle->addAlternativeName ( "LithoStaticPressure", "LithoStaticpressure" );
   projectHandle->addAlternativeName ( "LithoStaticPressure", "Lithostaticpressure" );
   projectHandle->addAlternativeName ( "LithoStaticPressure", "lithostaticpressure" );

   projectHandle->addAlternativeName ( "Ves", "ves" );

   projectHandle->addAlternativeName ( "MaxVes", "Maxves" );
   projectHandle->addAlternativeName ( "MaxVes", "maxves" );

   projectHandle->addAlternativeName ( "Porosity", "porosity" );

   projectHandle->addAlternativeName ( "Permeability", "permeability" );
   projectHandle->addAlternativeName ( "Permeability", "PermeabilityN" );
   projectHandle->addAlternativeName ( "Permeability", "permeabilityN" );
   projectHandle->addAlternativeName ( "Permeability", "permN" );

   projectHandle->addAlternativeName ( "HorizontalPermeability", "horizontalpermeability" );
   projectHandle->addAlternativeName ( "HorizontalPermeability", "permeabilityH" );
   projectHandle->addAlternativeName ( "HorizontalPermeability", "PermeabilityH" );
   projectHandle->addAlternativeName ( "HorizontalPermeability", "permH" );

   projectHandle->addAlternativeName ( "BulkDensity", "Bulkdensity" );
   projectHandle->addAlternativeName ( "BulkDensity", "bulkdensity" );
}


void addDefaultProperties ( const Mining::ProjectHandle* projectHandle,
                            DataMiner::PropertySet& properties ) {

   unsigned int i;

   // Fixed set of properties for output.
   properties.push_back ( projectHandle->findProperty ( "Temperature" ));
   properties.push_back ( projectHandle->findProperty ( "Vr" ));
   properties.push_back ( projectHandle->findProperty ( "HydroStaticPressure" ));
   properties.push_back ( projectHandle->findProperty ( "Pressure" ));
   properties.push_back ( projectHandle->findProperty ( "OverPressure" ));
   properties.push_back ( projectHandle->findProperty ( "LithoStaticPressure" ));
   properties.push_back ( projectHandle->findProperty ( "FracturePressure" ));

   properties.push_back ( projectHandle->findProperty ( "Ves" ));
   properties.push_back ( projectHandle->findProperty ( "MaxVes" ));
   properties.push_back ( projectHandle->findProperty ( "Porosity" ));
   properties.push_back ( projectHandle->findProperty ( "Permeability" ));
   properties.push_back ( projectHandle->findProperty ( "HorizontalPermeability" ));
   properties.push_back ( projectHandle->findProperty ( "ThermalConductivity" ));
   properties.push_back ( projectHandle->findProperty ( "ThermalConductivityH" ));

   properties.push_back ( projectHandle->findProperty ( "BulkDensity" ));

   properties.push_back ( projectHandle->findProperty ( "BrineViscosity" ));
   properties.push_back ( projectHandle->findProperty ( "BrineDensity" ));

   // properties.push_back ( projectHandle->findProperty ( "HeatFlowX" ));
   // properties.push_back ( projectHandle->findProperty ( "HeatFlowY" ));
   properties.push_back ( projectHandle->findProperty ( "HeatFlowZ" ));
   properties.push_back ( projectHandle->findProperty ( "HeatFlowMagnitude" ));
   properties.push_back ( projectHandle->findProperty ( "RadiogenicHeatProduction" ));

   properties.push_back ( projectHandle->findProperty ( "Velocity" ));
   properties.push_back ( projectHandle->findProperty ( "FluidVelocityX" ));
   properties.push_back ( projectHandle->findProperty ( "FluidVelocityY" ));
   properties.push_back ( projectHandle->findProperty ( "FluidVelocityZ" ));
   properties.push_back ( projectHandle->findProperty ( "FluidVelocityMagnitude" ));

   // properties.push_back ( projectHandle->findProperty ( "ThCond" ));
   // properties.push_back ( projectHandle->findProperty ( "ThermalConductivityH" ));

   properties.push_back ( projectHandle->findProperty ( "TemperatureGradient" ));
   properties.push_back ( projectHandle->findProperty ( "BasinTemperatureGradient" ));
   properties.push_back ( projectHandle->findProperty ( "BasementHeatFlow" ));

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      properties.push_back ( projectHandle->findProperty ( pvtFlash::ComponentIdNames [ i ] + "Concentration" ));
   }

   properties.push_back ( projectHandle->findProperty ( "WaterSaturation" ));
   properties.push_back ( projectHandle->findProperty ( "OilSaturation" ));
   properties.push_back ( projectHandle->findProperty ( "GasSaturation" ));


}
