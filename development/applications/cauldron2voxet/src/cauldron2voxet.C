#include <vector>
#include <cmath>
#include <stdlib.h>
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
#include "../Interface/ProjectHandle.h"

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
void createVoxetProjectFile ( Interface::ProjectHandle* cauldronProject, ostream & outputStream, const Snapshot * snapshot );


/// Write the values to the specified file.
void write ( const std::string& name,
            const VoxetPropertyGrid& values );

/// Correct the endian-ness of the array. The voxet format requires that the binary data is written 
/// in big-endian format. After this call the numbers will be un-usable in the code.
void correctEndian ( VoxetPropertyGrid& values );

bool splitString (char * string, char separator, char * & firstPart, char * & secondPart, char * & thirdPart);
double selectDefined (double undefinedValue, double preferred, double alternative);

bool useBasement = true;
bool verbose = false;
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
            showUsage ("Argument for '-snapshot' is missing");
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
            showUsage ("Argument for '-snapshot' is missing");
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
            showUsage ("Argument for '-snapshot' is missing");
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
      else if (strncmp (argv[arg], "-debug", Max (4, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if (strncmp (argv[arg], "-help", Max (2, strlen (argv[arg]))) == 0)
      {
         showUsage (" Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-?", Max (2, strlen (argv[arg]))) == 0)
      {
         showUsage (" Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-usage", Max (2, strlen (argv[arg]))) == 0)
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

   Interface::ProjectHandle * projectHandle = (Interface::ProjectHandle *) (OpenCauldronProject (projectFileName, "r"));

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

      createVoxetProjectFile (projectHandle, voxetProjectFileStream, snapshot);
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

      createVoxetProjectFile (projectHandle, voxetProjectFileStream, snapshot);
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

   VoxetCalculator vc (projectHandle, voxetProject->getGridDescription ());
   if (useBasement && verbose) cout << "Using basement" << endl;
   vc.useBasement() = useBasement;

   vc.setDepthProperty (depthProperty);


   // Array used to store interpolated values, this will then be output to file.
   VoxetPropertyGrid interpolatedProperty (voxetProject->getGridDescription ());

   asciiOutputFile.flags (ios::fixed);

   asciiOutputFile << "GOCAD Voxet 1.0" << endl;
   asciiOutputFile << "HEADER" << endl;
   asciiOutputFile << "{" << endl;
   asciiOutputFile << "name:" << outputFileName << endl;
   asciiOutputFile << "}" << endl;

   asciiOutputFile << "GOCAD_ORIGINAL_COORDINATE_SYSTEM" << endl;
   asciiOutputFile << "NAME Default" << endl;
   asciiOutputFile << "AXIS_NAME \"X\" \"Y\" \"Z\" " << endl;
   asciiOutputFile << "AXIS_UNIT \"m\" \"m\" \"m\" " << endl;
   asciiOutputFile << "ZPOSITIVE Depth" << endl;
   asciiOutputFile << "END_ORIGINAL_COORDINATE_SYSTEM" << endl;

   asciiOutputFile << "AXIS_O "
                   << gridDescription.getVoxetGridOrigin (0) << "  "
                   << gridDescription.getVoxetGridOrigin (1) << "  "
                   << gridDescription.getVoxetGridOrigin (2) << "  " << endl;


   asciiOutputFile << "AXIS_U " << gridDescription.getVoxetGridMaximum (0) - gridDescription.getVoxetGridOrigin (0) << "  " << " 0.0  0.0 " << endl;
   asciiOutputFile << "AXIS_V  0.0 " << gridDescription.getVoxetGridMaximum (1) - gridDescription.getVoxetGridOrigin (1) << "  " << " 0.0  " << endl;
   asciiOutputFile << "AXIS_W 0.0 0.0  " << gridDescription.getVoxetGridMaximum (2) - gridDescription.getVoxetGridOrigin (2) << "  " << endl;
   asciiOutputFile << "AXIS_MIN 0.0 0.0 0.0 " << endl;
   asciiOutputFile << "AXIS_MAX 1  1  1" << endl;

   asciiOutputFile << "AXIS_N "
                   << gridDescription.getVoxetNodeCount (0) << "  "
                   << gridDescription.getVoxetNodeCount (1) << "  " << gridDescription.getVoxetNodeCount (2) << "  " << endl;

   asciiOutputFile << "AXIS_NAME \"X\" \"Y\" \"Z\" " << endl;
   asciiOutputFile << "AXIS_UNIT \"m\" \"m\" \"m\" " << endl;
   asciiOutputFile << "AXIS_TYPE even even even" << endl;

   asciiOutputFile << endl;


   CauldronPropertyList::iterator cauldronPropIter;
   for (cauldronPropIter = voxetProject->cauldronPropertyBegin (); cauldronPropIter != voxetProject->cauldronPropertyEnd (); ++cauldronPropIter)
   {
      const Property *property = (*cauldronPropIter)->getProperty ();

      if (!property)
      {
         cerr << " Unknown property: " << (*cauldronPropIter)->getCauldronName () << endl;
         continue;
      }

      PropertyValueList *propertyValueList = projectHandle->getPropertyValues (FORMATION, property, snapshot, 0, 0, 0, VOLUME);
      unsigned int size = propertyValueList->size ();

      delete propertyValueList;

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

         asciiOutputFile << "PROPERTY " << propertyCount << "  \"" << (*cauldronPropIter)->getVoxetName () << '"' << endl;
         asciiOutputFile << "PROPERTY_KIND " << propertyCount << "  \"" << (*cauldronPropIter)->getVoxetName () << '"' << endl;
	 asciiOutputFile << "PROPERTY_CLASS " << propertyCount << " \"" << (*cauldronPropIter)->getVoxetName () << '"' << endl;
	 asciiOutputFile << "PROPERTY_CLASS_HEADER " << propertyCount << " \"" << (*cauldronPropIter)->getVoxetName () << "\" {" << endl;
	 asciiOutputFile << "name:" << (*cauldronPropIter)->getVoxetName () << endl << "}" << endl;

	 if ((*cauldronPropIter)->getCauldronName () == "Depth")
	 {
	    asciiOutputFile << "PROPERTY_SUBCLASS " << propertyCount << " " << "LINEARFUNCTION Float -1 0" << endl;
	 }
	 else
	 {
	    asciiOutputFile << "PROPERTY_SUBCLASS " << propertyCount << " " << "QUANTITY Float" << endl;
	 }

         asciiOutputFile << "PROP_ORIGINAL_UNIT " << propertyCount << " " << (*cauldronPropIter)->getUnits () << endl;
         asciiOutputFile << "PROP_UNIT " << propertyCount << " " << (*cauldronPropIter)->getUnits () << endl;
         asciiOutputFile << "PROP_ESIZE " << propertyCount << " " << sizeof (float) << endl;
         asciiOutputFile << "PROP_ETYPE " << propertyCount << " IEEE " << endl;
         asciiOutputFile << "PROP_NO_DATA_VALUE " << propertyCount << " " << vc.getNullValue (property) << endl;
         asciiOutputFile << "PROP_FILE " << propertyCount << " " << propertyFileName << endl;
	 asciiOutputFile << endl;

         vc.computeProperty (*cauldronPropIter, interpolatedProperty, verbose);
         correctEndian (interpolatedProperty);
         write (propertyFileName, interpolatedProperty);
         ++propertyCount;

         if ( verbose ) {
            cout << " deleting interpolators for property: " << property->getName () << endl;
         }

         vc.deleteProperty (property);

      }

   }

   if (verbose)
   {
      cout << endl;
   }

   asciiOutputFile.close ();
   CloseCauldronProject (projectHandle);

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
         << "                  [-nobasement]" << endl
         << "                  [-verbose]" << endl
         << "                  [-help]" << endl
         << "                  [-?]" << endl
         << "                  [-usage]" << endl
         << endl
         << "    -project           The cauldron project file." << endl
         << "    -spec              Use the specified spec file. Use a standard spec file if missing." << endl
         << "    -snapshot          Use the specified snapshot age. Not valid in conjunction with '-spec'," << endl
	 << "    -origin            Use the specified coordinates as the origin of the sample cube" << endl
	 << "    -delta             Use the specified values as the sampling distance in the x, y and z direction" << endl
	 << "    -count             Use the specified values as the number of samples in the x, y and z direction" << endl
         << "    -output            Output voxet file-name, MUST NOT contain the .vo extension, this will be added." << endl
         << "    -create-spec       Write a standard spec file into the specified file name," << endl
         << "                       the cauldron project file must also be specified." << endl
         << "    -nobasement        Ignore basement layers." << endl
         << "    -verbose           Generate some extra output." << endl
         << "    -help              Print this message." << endl
         << "    -?                 Print this message." << endl
         << "    -usage             Print this message." << endl << endl;
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


void createVoxetProjectFile ( Interface::ProjectHandle* cauldronProject, ostream &outputStream, const Snapshot * snapshot )
{
   char * propertyNames [] =
   {
      "Depth", "Pressure", "OverPressure", "HydroStaticPressure", "LithoStaticPressure", "Temperature", "Vr", "Ves", "MaxVes", "Porosity", "Permeability", "BulkDensity", ""
   };

   char * units [] =
   {
      "m", "MPa", "MPa", "MPa", "MPa", "degC", "percent", "Pa", "Pa", "percent", "mD", "kg/m^3", ""
   };

   double conversions [] =
   {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
   };

   char * outputPropertyNames [] =
   {
      "BPA_Depth", "BPA_Pressure", "BPA_OverPressure", "BPA_HydrostaticPressure", "BPA_LithoStaticPressure", "BPA_Temperature", "BPA_Vr", "BPA_Ves", "BPA_MaxVes", "BPA_Porosity", "BPA_Permeability", "BPA_BulkDensity", ""
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

   PropertyValueList* bottomDepthPropertyValueList = cauldronProject->getPropertyValues (SURFACE, depthProperty, snapshot, 0, 0, bottomSurface, MAP);
   if (bottomDepthPropertyValueList->size () != 1)
   {
      cerr << "Illegal number (" << bottomDepthPropertyValueList->size () << ") of depth property values for bottom surface " << bottomSurface->getName () << " at snapshot " << snapshot->getTime () << endl;
      return;
   }
   const GridMap *bottomDepthGridMap = bottomDepthPropertyValueList->front ()->getGridMap ();
   const GridMap *topDepthGridMap = 0;
      
   Interface::SurfaceList::iterator surfaceIter;
   for ( surfaceIter = surfaces->begin (); topDepthGridMap == 0 && surfaceIter != surfaces->end (); ++surfaceIter )
   {
      const Interface::Surface * topSurface = *surfaceIter;

      PropertyValueList* topDepthPropertyValueList = cauldronProject->getPropertyValues (SURFACE, depthProperty, snapshot, 0, 0, topSurface, MAP);
      if (topDepthPropertyValueList->size () != 1)
      {
         // cerr << "Illegal number (" << topDepthPropertyValueList->size () << ") of depth property values for top surface " << topSurface->getName () << " at snapshot " << snapshot->getTime () << endl;
         delete topDepthPropertyValueList;
         continue;
      }

      topDepthGridMap = topDepthPropertyValueList->front ()->getGridMap ();
      delete topDepthPropertyValueList;
      break;
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
