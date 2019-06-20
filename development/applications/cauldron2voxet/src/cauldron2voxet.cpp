//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <vector>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <values.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;

// DataAccess API includes
#include "GridMap.h"
#include "Grid.h"
#include "Snapshot.h"
#include "Surface.h"
#include "Formation.h"
#include "Reservoir.h"
#include "LithoType.h"
#include "Property.h"
#include "PropertyValue.h"
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "SimulationDetails.h"

// GeoPhysics library
#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "SurfaceProperty.h"
#include "PrimarySurfaceProperty.h"

#include "DepthInterpolator.h"
#include "LayerInterpolator.h"
#include "GridDescription.h"
#include "VoxetCalculator.h"
#include "VoxetPropertyGrid.h"
#include "array.h"

#include "CauldronProperty.h"
#include "VoxetProjectHandle.h"
#include "voxetschema.h"
#include "voxetschemafuncs.h"

#include <string>
#include <vector>
using namespace std;

using namespace DataAccess;
using namespace Interface;

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)

const double CauldronUndefined = 99999;
const double EclipseUndefined = -9999;

static char * argv0 = 0;
static void showUsage (const char * message = 0);
static void changeUndefinedValue (double & var, double oldValue, double newValue);

double originX = MAXDOUBLE, originY = MAXDOUBLE, originZ = MAXDOUBLE;
double deltaX = MAXDOUBLE, deltaY = MAXDOUBLE, deltaZ = MAXDOUBLE;
double countX = MAXDOUBLE, countY = MAXDOUBLE, countZ = MAXDOUBLE;

/// Print to stdout a default voxet file based on the cauldron project file that has been input.
void createVoxetProjectFile ( Interface::ProjectHandle* cauldronProject,
                              DerivedProperties::DerivedPropertyManager& propertyManager,
                              ostream & outputStream, const Snapshot * snapshot );


/// Write the values to the specified file.
void write ( const std::string& name,
            const VoxetPropertyGrid& values );

/// Correct the endian-ness of the array. The voxet format requires that the binary data is written
/// in big-endian format. After this call the numbers will be un-usable in the code.
void correctEndian ( VoxetPropertyGrid& values );

bool splitString (char * string, char separator, char * & firstPart, char * & secondPart, char * & thirdPart);
double selectDefined (double undefinedValue, double preferred, double alternative);

/// write the ascii voxet header (vo-file)
void writeVOheader(       ofstream& file,
                    const GridDescription & gridDescription,
                    const string& outputFileName);

/// write the property information into the voxet header (vo-file)
void writeVOproperty(       ofstream& file,
                      const int& propertyCount,
                      const CauldronProperty* cauldronProperty,
                      const string& propertyFileName,
                      const float& nullValue);

/// write the tail of the  ascii voxet header (vo-file)
void writeVOtail( ofstream& file);

bool useBasement = true;
bool verbose = false;
bool singlePropertyHeader = false;
bool debug = false;

int main (int argc, char ** argv)
{
   string projectFileName;
   string voxetFileName;
   string createVoxetFileName;
   string outputFileName;

   int numberOfInterpolatedXNodes = 101;
   int numberOfInterpolatedYNodes = 101;
   int numberOfInterpolatedZNodes = 41;

   float interpolatedZOrigin = 0.0;
   float interpolatedZEnd = 4500.0;

   std::map<std::string, double > propertyNullValueReplaceLookup = std::map<std::string, double >();

   if ((argv0 = strrchr (argv[0], '/')) != 0)
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }

   double snapshotTime = 0;
   int arg;

   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-output", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-output' is missing");
            return -1;
         }
         outputFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-project", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-project' is missing");
            return -1;
         }
         projectFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-spec", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-spec' is missing");
            return -1;
         }
         voxetFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-snapshot", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-snapshot' is missing");
            return -1;
         }
         snapshotTime = atof (argv[++arg]);
      }
      else if (strncmp (argv[arg], "-origin", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-origin' is missing");
            return -1;
         }
         char * c_origins = argv[++arg];
	 char * c_originX;
	 char * c_originY;
	 char * c_originZ;

	 splitString (c_origins, ',', c_originX, c_originY, c_originZ);

	 if (c_originX) originX = atof (c_originX);
	 if (c_originY) originY = atof (c_originY);
	 if (c_originZ) originZ = atof (c_originZ);
      }
      else if (strncmp (argv[arg], "-delta", Max (4, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-delta' is missing");
            return -1;
         }
         char * c_deltas = argv[++arg];
	 char * c_deltaX;
	 char * c_deltaY;
	 char * c_deltaZ;

	 splitString (c_deltas, ',', c_deltaX, c_deltaY, c_deltaZ);

	 if (c_deltaX) deltaX = atof (c_deltaX);
	 if (c_deltaY) deltaY = atof (c_deltaY);
	 if (c_deltaZ) deltaZ = atof (c_deltaZ);
      }
      else if (strncmp (argv[arg], "-count", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-count' is missing");
            return -1;
         }
         char * c_counts = argv[++arg];
	 char * c_countX;
	 char * c_countY;
	 char * c_countZ;

	 splitString (c_counts, ',', c_countX, c_countY, c_countZ);

	 if (c_countX) countX = atof (c_countX);
	 if (c_countY) countY = atof (c_countY);
	 if (c_countZ) countZ = atof (c_countZ);
      }
      else if (strncmp (argv[arg], "-nullvaluereplace", Max (5, strlen (argv[arg]))) == 0){
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-nullvaluereplace' is missing");
            return -1;
         }
         while (arg+1<argc && strncmp( argv[arg+1],"-",1)!=0 ){
            char * c_nullValueReplaceOption = argv[++arg];
            char * nullValueReplaceName;
            char * nullValueReplaceValue;
            char * tmp;
            splitString (c_nullValueReplaceOption,',',nullValueReplaceName,nullValueReplaceValue,tmp);
            if(!nullValueReplaceName || !nullValueReplaceValue){
               showUsage ("Argument for '-nullvalueeplace' wrong format");
               return -1;
            }
            propertyNullValueReplaceLookup.insert(std::pair<std::string,double>(std::string(nullValueReplaceName),
                                                                                atof (nullValueReplaceValue)));
         }


      }
      else if (strncmp (argv[arg], "-debug", Max (4, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if ((strncmp (argv[arg], "-help",  Max (2, strlen (argv[arg]))) == 0) ||
	           (strncmp (argv[arg], "-?",     Max (2, strlen (argv[arg]))) == 0) ||
			   (strncmp (argv[arg], "-usage", Max (2, strlen (argv[arg]))) == 0))
      {
         showUsage (" Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-nobasement", Max (4, strlen (argv[arg]))) == 0)
      {
         useBasement = false;
      }
      else if (strncmp (argv[arg], "-verbose", Max (4, strlen (argv[arg]))) == 0)
      {
         verbose = true;
      }
      else if (strncmp (argv[arg], "-singlepropertyheader", Max (4, strlen (argv[arg]))) == 0)
      {
         singlePropertyHeader = true;
      }
      else if (strncmp (argv[arg], "-create-spec", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-create-spec' is missing");
            return -1;
         }
         createVoxetFileName = argv[++arg];
      }
      else
      {

         char errorMessage[256];

         sprintf (errorMessage, " Illegal argument: %s", argv[arg]);
         showUsage (errorMessage);
         return -1;
      }
   }

   if (projectFileName == "")
   {
      showUsage ("No project file specified");
      return -1;
   }

   if (outputFileName == "")
   {
      size_t dotPos = projectFileName.find (".project");

      outputFileName = projectFileName.substr (0, dotPos);
   }

   GeoPhysics::ObjectFactory* factory = new GeoPhysics::ObjectFactory;
   GeoPhysics::ProjectHandle* projectHandle = dynamic_cast< GeoPhysics::ProjectHandle* >( OpenCauldronProject( projectFileName, "r", factory ) );
   DerivedProperties::DerivedPropertyManager propertyManager ( projectHandle );

   bool coupledCalculationMode = false;
   bool started = projectHandle->startActivity ( "cauldron2voxet", projectHandle->getLowResolutionOutputGrid (), false, false, false );

   if ( not started ) {
      return 1;
   }

   const Interface::SimulationDetails* simulationDetails = projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

   if ( simulationDetails != 0 ) {
      coupledCalculationMode = simulationDetails->getSimulatorMode () == "Overpressure" or
                               simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" or
                               simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" or
                               simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" or
                               simulationDetails->getSimulatorMode () == "CoupledDarcy";
   } else {
      // If this table is not present the assume that the last
      // fastcauldron mode was not pressure mode.
      // This table may not be present because we are running c2e on an old
      // project, before this table was added.
      coupledCalculationMode = false;
   }

   started = projectHandle->initialise ( coupledCalculationMode );

   if ( not started ) {
      return 1;
   }

   started = projectHandle->setFormationLithologies ( true, true );

   if ( not started ) {
      return 1;
   }


   const Snapshot *snapshot = projectHandle->findSnapshot (snapshotTime);

   if (!snapshot)
   {
      cerr << "No calculations have been made for snapshot time " << snapshotTime << endl;
      return -1;
   }

   const Grid *cauldronGrid = projectHandle->getLowResolutionOutputGrid ();

   if (createVoxetFileName != "")
   {
      ofstream voxetProjectFileStream;

      voxetProjectFileStream.open (createVoxetFileName.c_str ());
      if (!voxetProjectFileStream.is_open ())
      {
         cerr << "Could not open temporary file " << createVoxetFileName << ", aborting ...." << endl;
         return -1;
      }

      createVoxetProjectFile (projectHandle, propertyManager, voxetProjectFileStream, snapshot);
      return 0;
   }

   VoxetProjectHandle *voxetProject = 0;

   if (voxetFileName == "")
   {
      ofstream voxetProjectFileStream;

      char tmpVoxetFileName[256];

      sprintf (tmpVoxetFileName, "/tmp/voxetProjectFile%d", getpid ());

      voxetProjectFileStream.open (tmpVoxetFileName);
      if (!voxetProjectFileStream.is_open ())
      {
         cerr << "Could not open temporary file " << tmpVoxetFileName << ", aborting ...." << endl;
         return -1;
      }

      createVoxetProjectFile (projectHandle, propertyManager, voxetProjectFileStream, snapshot);
      voxetProjectFileStream.close ();

      voxetProject = new VoxetProjectHandle (tmpVoxetFileName, projectHandle);
      unlink (tmpVoxetFileName);
   }
   else
   {
      voxetProject = new VoxetProjectHandle (voxetFileName, projectHandle);
   }

   snapshot = projectHandle->findSnapshot (voxetProject->getSnapshotTime ());

   if (!voxetProject->isConsistent ())
   {
      cout << " Voxet file is not consistent. " << endl;
      exit (-1);
   }


   string asciiFileName = outputFileName + ".vo";
   if(singlePropertyHeader){
      asciiFileName = outputFileName + "_all.vo";
   }
   string binaryFileName = outputFileName;

   FILE *binaryOutputFile;

   ofstream asciiOutputFile;

   asciiOutputFile.open (asciiFileName.c_str ());

   if (asciiOutputFile.fail ())
   {
      cerr << "Could not open output file " << asciiFileName << endl;
      return -1;
   }



   if (projectFileName.length () == 0)
   {
      cerr << "Could not open project file " << projectFileName << endl;
      return -1;
   }

   if (verbose)
   {
      cout << "Using snapshot " << setprecision (10) << snapshot->getTime () << endl;
   }

   if (verbose)
   {
      cout << endl;
   }

   const Property *depthProperty = projectHandle->findProperty ("Depth");

   if (!depthProperty)
   {
      cerr << "Could not find the Depth property in the project file " << endl << "Are you sure the project file contains output data?" << endl;
      return -1;
   }

   const GridMap *depthGridMap = 0;
   const GridMap *vesGridMap = 0;

   int propertyCount = 1;

   const GridDescription & gridDescription = voxetProject->getGridDescription ();

   VoxetCalculator vc (projectHandle, propertyManager, voxetProject->getGridDescription (),propertyNullValueReplaceLookup);

   if (useBasement && verbose) cout << "Using basement" << endl;
   vc.useBasement() = useBasement;

   vc.setDepthProperty (depthProperty);


   // Array used to store interpolated values, this will then be output to file.
   VoxetPropertyGrid interpolatedProperty (voxetProject->getGridDescription ());

   asciiOutputFile.flags (ios::fixed);

   writeVOheader(asciiOutputFile, gridDescription, outputFileName);

   CauldronPropertyList::iterator cauldronPropIter;
   for (cauldronPropIter = voxetProject->cauldronPropertyBegin (); cauldronPropIter != voxetProject->cauldronPropertyEnd (); ++cauldronPropIter)
   {
      const Property *property = (*cauldronPropIter)->getProperty ();

      if (!property)
      {
         cerr << " Unknown property: " << (*cauldronPropIter)->getCauldronName () << endl;
         continue;
      }


      // Check if the property has values (only for 2016.11 release. Remove this check to enable DerivedProperty calculation)
      PropertyValueList *propertyValueListAvailable = projectHandle->getPropertyValues (FORMATION, property, snapshot, 0, 0, 0, VOLUME);
      unsigned int propertiesSize = propertyValueListAvailable->size ();
      delete propertyValueListAvailable;
      if (propertiesSize == 0)
      {
         continue;
      }

      AbstractDerivedProperties::FormationPropertyList propertyValueList ( propertyManager.getFormationProperties ( property, snapshot, useBasement ));

      // Could just ask if property is computable.
      unsigned int size = propertyValueList.size ();

      if (size == 0)
      {
         continue;
      }

      if (verbose)
      {
         cout << " Adding cauldron property: " << property->getName () << endl;
      }

      if ((*cauldronPropIter)->getVoxetOutput ())
      {

         vc.addProperty (property);

         if (vc.computeInterpolators (snapshot, verbose) == -1)
         {
            cerr << " Are there any results in the project? " << endl;
            return -1;
         }

	 string propertyFileName = binaryFileName + "_" + (*cauldronPropIter)->getCauldronName () + "@@";

         writeVOproperty(asciiOutputFile, propertyCount, *cauldronPropIter, propertyFileName, vc.getNullValue(property) );

         vc.computeProperty (*cauldronPropIter, interpolatedProperty, verbose);
         correctEndian (interpolatedProperty);
         write (propertyFileName, interpolatedProperty);
         ++propertyCount;

         if ( verbose ) {
            cout << " deleting interpolators for property: " << property->getName () << endl;
         }

         if(singlePropertyHeader){
            ofstream asciiHeaderOutputFile;
            string asciiHeaderFileName=outputFileName + "_" + (*cauldronPropIter)->getCauldronName ()+".vo";

            asciiHeaderOutputFile.open (asciiHeaderFileName.c_str ());

            if (asciiHeaderOutputFile.fail ())
            {
               cerr << "Could not open output file " << asciiHeaderFileName << endl;
               return -1;
            }

            asciiHeaderOutputFile.flags (ios::fixed);

            writeVOheader(asciiHeaderOutputFile, gridDescription, outputFileName);

            writeVOproperty(asciiHeaderOutputFile, 1, *cauldronPropIter, propertyFileName, vc.getNullValue(property) );

            writeVOtail(asciiHeaderOutputFile);

            asciiHeaderOutputFile.close ();

         }

         vc.deleteProperty (property);

      }

   }

   if (verbose)
   {
      cout << endl;
   }
   writeVOtail(asciiOutputFile);
   asciiOutputFile.close ();
   CloseCauldronProject (projectHandle);
   delete factory;

   if (debug)
      cerr << "Project closed" << endl;

   return 0;
}

void showUsage (const char * message)
{
   cerr << endl;
   if (message)
   {
      cerr << argv0 << ": " << message << endl;
   }

   cerr << "Usage (Options may be abbreviated): " << endl
         << argv0 << "    -project <cauldron-project-file>" << endl
         << "                  [-spec <spec-file>]" << endl
         << "                  [-snapshot <age>]" << endl
	 << "                  [-origin <originX>,<originY>,<originZ>]" << endl
	 << "                  [-delta <deltaX>,<deltaY>,<deltaZ>]" << endl
	 << "                  [-count <countX>,<countY>,<countZ>]" << endl
         << "                  [-output <output-file-name>]" << endl
         << "                  [-create-spec <spec-file>]" << endl
         << "                  [-nullvaluereplace <PropertyName,Value> [<PropertyName,Value>] [...]]" << endl
         << "                  [-nobasement]" << endl
         << "                  [-propertyHeader]" << endl
         << "                  [-verbose]" << endl
         << "                  [-help]" << endl
         << "                  [-?]" << endl
         << "                  [-usage]" << endl
         << endl
         << "    -project              The cauldron project file." << endl
         << "    -spec                 Use the specified spec file. Use a standard spec file if missing." << endl
         << "    -snapshot             Use the specified snapshot age. Not valid in conjunction with '-spec'," << endl
	 << "    -origin               Use the specified coordinates as the origin of the sample cube" << endl
	 << "    -delta                Use the specified values as the sampling distance in the x, y and z direction" << endl
	 << "    -count                Use the specified values as the number of samples in the x, y and z direction" << endl
         << "    -output               Output voxet file-name, MUST NOT contain the .vo extension, this will be added." << endl
         << "    -create-spec          Write a standard spec file into the specified file name," << endl
         << "                          the cauldron project file must also be specified." << endl
         << "    -nullvaluereplace     Replace null values of the property by a given value." << endl
         << "    -nobasement           Ignore basement layers." << endl
         << "    -singlepropertyheader Writes one header file for each property. (additional to the multiple property header-file)" << endl
         << "    -verbose              Generate some extra output." << endl
         << "    -help                 Print this message." << endl
         << "    -?                    Print this message." << endl
         << "    -usage                Print this message." << endl << endl;
   exit (-1);
}

static void changeUndefinedValue (double & var, double oldValue, double newValue)
{
   if (var == oldValue) var = newValue;
}


void write ( const std::string& name,
            const VoxetPropertyGrid& values ) {

   FILE* file;

   file = fopen ( name.c_str (), "w" );

   if ( file == 0 ) {
      cerr << " cannot open file: " << name << " for writing" << endl;
      return;
   }

   fwrite ( values.getOneDData (), sizeof ( float ), values.getGridDescription ().getVoxetNodeCount (), file );
   fclose ( file );
}


float correctEndian ( const float x ) {

   union ByteConverter {
      float value;
      char bytes [ 4 ];
   };

   ByteConverter bc;
   ByteConverter result;

   bc.value = x;
   result.bytes [ 0 ] = bc.bytes [ 3 ];
   result.bytes [ 1 ] = bc.bytes [ 2 ];
   result.bytes [ 2 ] = bc.bytes [ 1 ];
   result.bytes [ 3 ] = bc.bytes [ 0 ];

   return result.value;
}

void correctEndian ( VoxetPropertyGrid& values )
{
   const int nx = values.getGridDescription ().getVoxetNodeCount (0);
   const int ny = values.getGridDescription ().getVoxetNodeCount (1);
   const int nz = values.getGridDescription ().getVoxetNodeCount (2);

   int i;
   int j;
   int k;

   for (k = 0; k < nz; ++k)
   {
      for (j = 0; j < ny; ++j)
      {
         for (i = 0; i < nx; ++i)
         {
            values (i, j, k) = correctEndian (values (i, j, k));
         }
      }
   }
}

bool splitString (char * string, char separator, char * & firstPart, char * & secondPart, char * & thirdPart)
{
   firstPart = 0;
   secondPart = 0;
   thirdPart = 0;
   char * tail;
   if (!string || strlen (string) == 0) return false;

   /***/

   firstPart = string;
   tail = strchr (firstPart, separator);

   if (tail == 0) return false;

   if (tail == firstPart) firstPart = 0;

   * tail = '\0';
   ++tail;
   if (* tail == '\0')return false;

   /***/

   secondPart = tail;
   tail = strchr (secondPart, separator);

   if (tail == 0) return false;

   if (tail == secondPart) secondPart = 0;

   * tail = '\0';
   ++tail;
   if (* tail == '\0') return false;

   /***/

   thirdPart = tail;

   return true;
}

double selectDefined (double undefinedValue, double preferred, double alternative)
{
   return (preferred != undefinedValue ? preferred : alternative);
}


void createVoxetProjectFile ( Interface::ProjectHandle* cauldronProject,
                              DerivedProperties::DerivedPropertyManager& propertyManager,
                              ostream &outputStream, const Snapshot * snapshot )
{
   char * propertyNames [] =
   {
      "Depth",
      "Pressure", "OverPressure", "HydroStaticPressure", "LithoStaticPressure",
      "Temperature", "Vr",
      "Ves", "MaxVes",
      "Porosity", "Permeability", "BulkDensity",
      "Velocity", "TwoWayTime",
      ""
   };

   char * units [] =
   {
      "m",
      "MPa", "MPa", "MPa", "MPa",
      "degC", "percent",
      "Pa", "Pa",
      "percent", "mD", "kg/m^3",
      "m/s", "ms",
      ""
   };

   double conversions [] =
   {
      1,
      1, 1, 1, 1,
      1, 1,
      1, 1,
      1, 1, 1,
      1, 1,
      0
   };

   char * outputPropertyNames [] =
   {
      "BPA_Depth",
      "BPA_Pressure", "BPA_OverPressure", "BPA_HydrostaticPressure", "BPA_LithoStaticPressure",
      "BPA_Temperature", "BPA_Vr",
      "BPA_Ves", "BPA_MaxVes",
      "BPA_Porosity", "BPA_Permeability", "BPA_BulkDensity",
      "BPA_Velocity", "BPA_TwoWayTime",
      ""
   };

   database::DataSchema* voxetSchema = database::createVoxetSchema ();
   database::Database*   database = database::Database::CreateFromSchema ( *voxetSchema );
   database::Table*      table;
   database::Record*     record;
   Interface::FormationList* formations;
   Interface::FormationList::iterator formationIter;
   const Interface::Grid * grid;

   //------------------------------------------------------------//

   table = database->getTable ( "CauldronPropertyIoTbl" );

   int p;
   for (p = 0; strlen (propertyNames[p]) != 0; ++p)
   {
      const Property * property = cauldronProject->findProperty (propertyNames[p]);
      if (property)
      {
	 record = table->createRecord ();
	 database::setCauldronPropertyName ( record, propertyNames[p] );
	 database::setVoxetPropertyName ( record, outputPropertyNames[p] );
	 database::setOutputPropertyUnits ( record, units[p]);
	 database::setConversionFactor ( record, conversions[p] );
	 database::setVoxetOutput ( record, 1 );
      }
   }

   //------------------------------------------------------------//

   table = database->getTable ( "SnapshotTimeIoTbl" );

   record = table->createRecord ();
   database::setSnapshotTime ( record, snapshot->getTime () );

   //------------------------------------------------------------//

   const Property *depthProperty = cauldronProject->findProperty ("Depth");

   if (!depthProperty)
   {
      cerr << "Could not find the Depth property in the project file " << endl << "Are you sure the project file contains output data?" << endl;
      return;
   }

   Interface::SurfaceList* surfaces = cauldronProject->getSurfaces ();
   const Interface::Surface * bottomSurface = surfaces->back ();

   AbstractDerivedProperties::SurfacePropertyPtr abstractBottomDepthPropertyValue = propertyManager.getSurfaceProperty ( depthProperty, snapshot, bottomSurface );
   auto bottomDepthPropertyValue = dynamic_pointer_cast<const DerivedProperties::PrimarySurfaceProperty>(abstractBottomDepthPropertyValue);

   if (bottomDepthPropertyValue == 0 )
   {
      cerr << " Depth property for bottom surface " << bottomSurface->getName () << " at snapshot " << snapshot->getTime () << " is not available." << endl;
      return;
   }

   const GridMap *bottomDepthGridMap = 0; // = bottomDepthPropertyValueList->front ()->getGridMap ();
   const GridMap *topDepthGridMap = 0;

   bottomDepthGridMap = bottomDepthPropertyValue.get ()->getGridMap ();

   if ( bottomDepthGridMap == 0 )
   {
      cerr << " Depth property for bottom surface " << bottomSurface->getName () << " at snapshot " << snapshot->getTime () << " is not available." << endl;
      return;
   }

   Interface::SurfaceList::iterator surfaceIter;
   for ( surfaceIter = surfaces->begin (); topDepthGridMap == 0 && surfaceIter != surfaces->end (); ++surfaceIter )
   {
      const Interface::Surface * topSurface = *surfaceIter;

      AbstractDerivedProperties::SurfacePropertyPtr abstractTopDepthPropertyValue = propertyManager.getSurfaceProperty ( depthProperty, snapshot, topSurface );
      auto topDepthPropertyValue = dynamic_pointer_cast<const DerivedProperties::PrimarySurfaceProperty>(abstractTopDepthPropertyValue);

      if ( topDepthPropertyValue == 0 )
      {
         cerr << " Depth property for top surface " << topSurface->getName () << " at snapshot " << snapshot->getTime () << " is not available." << endl;
         continue;
      }

      topDepthGridMap = topDepthPropertyValue.get ()->getGridMap ();
      break;
   }


   if ( topDepthGridMap == 0 )
   {
      cerr << " Depth property for top surface " << " is not available." << endl;
      return;
   }

   double minimumDepth;
   double maximumDepth;
   double dummyDepth;

   topDepthGridMap->getMinMaxValue (minimumDepth, dummyDepth);
   bottomDepthGridMap->getMinMaxValue (dummyDepth, maximumDepth);

   //------------------------------------------------------------//

   grid = cauldronProject->getLowResolutionOutputGrid ();
   table = database->getTable ( "VoxetGridIoTbl" );

   const double deltaK = 100;

   record = table->createRecord ();
   database::setVoxetOriginX ( record, selectDefined (MAXDOUBLE, originX, grid->minI ()) );
   database::setVoxetOriginY ( record, selectDefined (MAXDOUBLE, originY, grid->minJ ()) );
   database::setVoxetOriginZ ( record, selectDefined (MAXDOUBLE, originZ, minimumDepth - 100));
   database::setVoxetDeltaX ( record, selectDefined (MAXDOUBLE, deltaX, grid->deltaI ()) );
   database::setVoxetDeltaY ( record, selectDefined (MAXDOUBLE, deltaY, grid->deltaJ ()) );
   database::setVoxetDeltaZ ( record, selectDefined (MAXDOUBLE, deltaZ, deltaK) );
   database::setNumberOfVoxetNodesX ( record, (int) selectDefined (MAXDOUBLE, countX, (double) grid->numI ()) );
   database::setNumberOfVoxetNodesY ( record, (int) selectDefined (MAXDOUBLE, countY, (double) grid->numJ ()) );
   database::setNumberOfVoxetNodesZ ( record, (int) selectDefined (MAXDOUBLE, countZ, ((maximumDepth - minimumDepth) / deltaK) + 3) );

   //------------------------------------------------------------//

   // Now write the stream to stdout.
   database->saveToStream ( outputStream );
}

void writeVOheader(       ofstream& file,
                    const GridDescription & gridDescription,
                    const string& outputFileName){

   file << "GOCAD Voxet 1.0" << endl;
   file << "HEADER" << endl;
   file << "{" << endl;
   file << "name:" << outputFileName << endl;
   file << "}" << endl;

   file << "GOCAD_ORIGINAL_COORDINATE_SYSTEM" << endl;
   file << "NAME Default" << endl;
   file << "AXIS_NAME \"X\" \"Y\" \"Z\" " << endl;
   file << "AXIS_UNIT \"m\" \"m\" \"m\" " << endl;
   file << "ZPOSITIVE Depth" << endl;
   file << "END_ORIGINAL_COORDINATE_SYSTEM" << endl;

   file << "AXIS_O "
                   << gridDescription.getVoxetGridOrigin (0) << "  "
                   << gridDescription.getVoxetGridOrigin (1) << "  "
                   << gridDescription.getVoxetGridOrigin (2) << "  " << endl;


   file << "AXIS_U " << gridDescription.getVoxetGridMaximum (0) - gridDescription.getVoxetGridOrigin (0) << "  " << " 0.0  0.0 " << endl;
   file << "AXIS_V  0.0 " << gridDescription.getVoxetGridMaximum (1) - gridDescription.getVoxetGridOrigin (1) << "  " << " 0.0  " << endl;
   file << "AXIS_W 0.0 0.0  " << gridDescription.getVoxetGridMaximum (2) - gridDescription.getVoxetGridOrigin (2) << "  " << endl;
   file << "AXIS_MIN 0.0 0.0 0.0 " << endl;
   file << "AXIS_MAX 1  1  1" << endl;

   file << "AXIS_N "
                   << gridDescription.getVoxetNodeCount (0) << "  "
                   << gridDescription.getVoxetNodeCount (1) << "  "
                   << gridDescription.getVoxetNodeCount (2) << "  " << endl;

   file << "AXIS_NAME \"X\" \"Y\" \"Z\" " << endl;
   file << "AXIS_UNIT \"m\" \"m\" \"m\" " << endl;
   file << "AXIS_TYPE even even even" << endl;

   file << endl;
}

void writeVOproperty(       ofstream& file,
                      const int& propertyCount,
                      const CauldronProperty* cauldronProperty,
                      const string& propertyFileName,
                      const float& nullValue){
   file << "PROPERTY " << propertyCount << "  \"" << cauldronProperty->getVoxetName () << '"' << endl;
   file << "PROPERTY_KIND " << propertyCount << "  \"" << cauldronProperty->getVoxetName () << '"' << endl;
   file << "PROPERTY_CLASS " << propertyCount << " \"" << cauldronProperty->getVoxetName () << '"' << endl;
   file << "PROPERTY_CLASS_HEADER " << propertyCount << " \"" << cauldronProperty->getVoxetName () << "\" {" << endl;
   file << "name:" << cauldronProperty->getVoxetName () << endl << "}" << endl;

   if (cauldronProperty->getCauldronName () == "Depth")
   {
      file << "PROPERTY_SUBCLASS " << propertyCount << " " << "LINEARFUNCTION Float -1 0" << endl;
   }
   else
   {
      file << "PROPERTY_SUBCLASS " << propertyCount << " " << "QUANTITY Float" << endl;
   }

   file << "PROP_ORIGINAL_UNIT " << propertyCount << " " << cauldronProperty->getUnits () << endl;
   file << "PROP_UNIT " << propertyCount << " " << cauldronProperty->getUnits () << endl;
   file << "PROP_ESIZE " << propertyCount << " " << sizeof (float) << endl;
   file << "PROP_ETYPE " << propertyCount << " IEEE " << endl;
   file << "PROP_NO_DATA_VALUE " << propertyCount << " " << nullValue << endl;
   file << "PROP_FILE " << propertyCount << " " << propertyFileName << endl;
   file << endl;
}

void writeVOtail( ofstream& file){
   file << "END"<<endl;
}
