#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <iostream>
#include <map>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "SurfaceProperty.h"

// DataAccess API includes
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"

#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

#include <string>
#include <vector>
using namespace std;

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

using database::Database;
using database::DataSchema;
using database::Table;
using database::Record;

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)


static char * argv0 = 0;
static void showUsage (const char * message = 0);

bool verbose = false;
bool debug = false;

// Possible operations
#define None	0
#define SaveTopDepth 1
#define CalcBelowMudLine 2

struct PropertyHandle
{
   string name;
   string header;
   string unit;
   double conversionFactor;
   int operation;
   const Property * property;
};

PropertyHandle propertyHandles[] =
{
   { "Temperature", "Temperature", "C", 1, None, 0 },
   { "Depth", "TVDSS", "m", 1, SaveTopDepth, 0 },
   { "Depth", "TVDBML", "m", 1, CalcBelowMudLine, 0 },
   { "Pressure", "Pressure", "MPa", 1, None, 0 },
   { "Ves", "Ves", "MPa", 1e-6, None, 0 },
};

const int NumPropertyHandles = 5;

static bool generateBHF (ProjectHandle * projectHandle, DerivedProperties::DerivedPropertyManager& propertyManager, Record * record);

int main (int argc, char ** argv)
{
   string inputProjectFileName;
   string outputProjectFileName;

   if ((argv0 = strrchr (argv[0], '/')) != 0)
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }
   
   int arg;
   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-save", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-save' is missing");
            return -1;
         }
         outputProjectFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-project", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-project' is missing");
            return -1;
         }
         inputProjectFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-debug", Max (2, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if (strncmp (argv[arg], "-verbose", Max (3, strlen (argv[arg]))) == 0)
      {
         verbose = true;
      }
      else if (strncmp (argv[arg], "-v2", 3) == 0)
      {
	 // catch the "-v" argument
      }
      else if (argv[arg][0] != '-')
      {
         inputProjectFileName = argv[arg];
      }
      else
      {
         showUsage ();
         return -1;
      }
   }

   if (inputProjectFileName == "")
   {
      showUsage ("No project file specified");
      return -1;
   }

   if (outputProjectFileName.length () == 0)
   {
      outputProjectFileName = inputProjectFileName;
   }

   
   ObjectFactory* factory = new ObjectFactory();
   GeoPhysics::ProjectHandle* projectHandle = dynamic_cast< GeoPhysics::ProjectHandle* >( OpenCauldronProject( inputProjectFileName, "rw", factory ) );
   DerivedProperties::DerivedPropertyManager propertyManager ( projectHandle );

   if (projectHandle == 0)
   {
      cerr << "Could not open project file " << inputProjectFileName << endl;

      if ( factory != 0 ) {
         delete factory;
      }

      return -1;
   }

   projectHandle->startActivity (argv0, projectHandle->getLowResolutionOutputGrid ());


   // Check for presence of properties
   int p;
   for (p = 0; p < NumPropertyHandles; ++p)
   {
      propertyHandles[p].property = projectHandle->findProperty (propertyHandles[p].name);

      if (!propertyHandles[p].property)
      {
	 cerr << "Could not find the " << propertyHandles[p].name << " property in the project file " << endl
	    << "Rerun pressure/temperature with the " << propertyHandles[p].name << " property turned on" << endl;
         delete projectHandle;
         delete factory;
	 return -1;

      }

      if ( not propertyManager.surfacePropertyIsComputable ( propertyHandles[p].property )) {
	 cerr << "Could not find the " << propertyHandles[p].name << " property in the project file " << endl
	    << "Rerun pressure/temperature with the " << propertyHandles[p].name << " property turned on" << endl;
         delete projectHandle;
         delete factory;
	 return -1;
      }

   }

   // iterate over the touchstone wells

   if (projectHandle->getModellingMode () == Interface::MODE3D)
   {
      Database * db = ((Interface::ProjectHandle *) projectHandle)->getDataBase ();
      Table * table = db->getTable ("TouchstoneWellIoTbl");
      int i;
      for (i = 0; i < table->size (); ++i)
      {
	 Record * record = table->getRecord (i);

	 generateBHF (projectHandle, propertyManager, record);
      }

      ((Interface::ProjectHandle *) projectHandle)->setSimulationDetails ( "cauldron2bhf", "Default", "" );
      ((Interface::ProjectHandle *) projectHandle)->finishActivity ();
      projectHandle->saveToFile (outputProjectFileName);
   }
   else
   {
      generateBHF (projectHandle, propertyManager, 0);
   }

   delete projectHandle;
   delete factory;
   return 0;
}

static bool generateBHF (ProjectHandle * projectHandle, DerivedProperties::DerivedPropertyManager& propertyManager, Record * record)
{
   bool is3D = (record != 0);

   double coordI = 0, coordJ = 0;
   string BHFName = projectHandle->getProjectName ();
   string wellName = projectHandle->getProjectName ();
   double easting = 0;
   double northing = 0;

   if (is3D)
   {
      wellName = database::getWellName (record);
      easting = database::getXCoord (record);
      northing = database::getYCoord (record);

      if (verbose)
	 cout << "Handling well: " << wellName << endl;

      // convert easting, northing to rational i, j
      const Grid *outputGrid = projectHandle->getLowResolutionOutputGrid ();

      if (outputGrid->getGridPoint (easting, northing, coordI, coordJ) == false)
      {
	 return false;
      }

      BHFName = wellName;
   }

   BHFName += ".BHF";

   if (is3D)
   {
      database::setBHFName (record, BHFName);
   }


   string outputFileName = projectHandle->getProjectPath ();

   if (is3D)
   {
      outputFileName =  projectHandle->getFullOutputDir ();
   }

   outputFileName += "/";
   outputFileName += BHFName;

   ofstream outputFile (outputFileName.c_str ());

   if (!outputFile.is_open ())
      return false;

   outputFile << setiosflags (ios::left);

   outputFile << setw (5) << "Well" << " = " << wellName
      << " (from Cauldron 3D project: " << projectHandle->getProjectName () << ")" << endl;
   outputFile << setw (15) << "* Easting" << " = " << easting << endl;
   outputFile << setw (15) << "* Northing" << " = " << northing << endl;

   SnapshotList *snapshots = projectHandle->getSnapshots ();

   FormationList *formations = projectHandle->getFormations ();

   const Formation * topMostFormation = formations->front ();
   const Surface *topMostSurface = topMostFormation->getTopSurface ();

   double topDepth = topMostSurface->getInputDepthMap()->getValue (coordI, coordJ);

   outputFile << setw (15) << "* Surface Depth" << " = " << topDepth << endl;

   map<const Snapshot *, double> surfaceDepths;
   map<const Snapshot *, bool> foundSurfaceDepths;

   SnapshotList::iterator snapshotIter;
   for (snapshotIter = snapshots->begin (); snapshotIter != snapshots->end (); ++snapshotIter)
   {
      const Snapshot *snapshot = *snapshotIter;
      foundSurfaceDepths[snapshot] = false;
   }

   FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      const Formation *formation = *formationIter;
      const Surface *topSurface = formation->getTopSurface ();
      const Surface *bottomSurface = formation->getBottomSurface ();

      if (verbose)
	 cout << "Handling formation: " << formation->getName () << endl;


      outputFile << endl;
      outputFile << setw (7) << "Unit = " << formation->getName () << endl;
      outputFile << "Type = Top/base" << endl;
      outputFile << "* " << endl;
      outputFile << "* Formation name = " << formation->getName ()
            << ". Top surface name = " << topSurface->getName ()
            << ". Bottom surface name = " << bottomSurface->getName () << "." << endl;

      outputFile << "* Mix model : " << database::getMixModel ((dynamic_cast<const Interface::Formation *>(formation))->getRecord ());

      const LithoType *lithotype;
      const GridMap *lithotypePercentageMap;

      if ((lithotype = formation->getLithoType1 ()) != 0 &&
          (lithotypePercentageMap = formation->getLithoType1PercentageMap ()) != 0)
      {
         outputFile << " : " << lithotype->getName ()
                    << "(" << lithotypePercentageMap->getValue (coordI, coordJ, 0) << "%)";
      }

      if ((lithotype = formation->getLithoType2 ()) != 0 &&
          (lithotypePercentageMap = formation->getLithoType2PercentageMap ()) != 0)
      {
         outputFile << ", " << lithotype->getName ()
	            << "(" << lithotypePercentageMap->getValue (coordI, coordJ, 0) << "%)";
      }

      if ((lithotype = formation->getLithoType3 ()) != 0 &&
          (lithotypePercentageMap = formation->getLithoType3PercentageMap ()) != 0)
      {
         outputFile << ", " << lithotype->getName ()
                    << "(" << lithotypePercentageMap->getValue (coordI, coordJ, 0) << "%)";
      }
      outputFile << endl;

      outputFile << "* Fluid type : "
                 << database::getFluidtype (( dynamic_cast<const Interface::Formation *>(formation))->getRecord ()) << endl;

      outputFile << "* " << endl;

      const short colW = 24;

      outputFile << setw (colW) << "* Time (Ma)";

      int p;

      for (p = 0; p < NumPropertyHandles; ++p)
      {
	 stringstream topHeader;
	 stringstream bottomHeader;
         topHeader <<  propertyHandles[p].header << "_t (" << propertyHandles[p].unit << ")";
         bottomHeader <<  propertyHandles[p].header << "_b (" << propertyHandles[p].unit << ")";
         outputFile << setw (colW) << topHeader.str ();
         outputFile << setw (colW) << bottomHeader.str ();
      }
      outputFile << endl;

      SnapshotList::iterator snapshotIter;
      for (snapshotIter = snapshots->begin (); snapshotIter != snapshots->end (); ++snapshotIter)
      {
         const Snapshot *snapshot = *snapshotIter;

	 if (verbose)
	    cout << "Handling snapshot: " << snapshot->getTime () << endl;

         // depth property values
         double topValues[NumPropertyHandles];
         double bottomValues[NumPropertyHandles];

	 bool failed = false;
         for (p = 0; p < NumPropertyHandles; ++p)
         {

            DerivedProperties::SurfacePropertyPtr topPropertyValues = propertyManager.getSurfaceProperty ( propertyHandles[p].property, snapshot, topSurface );

            if ( topPropertyValues == 0 )
	    {
	       failed = true;
	       continue;
	    }

            DerivedProperties::SurfacePropertyPtr bottomPropertyValues = propertyManager.getSurfaceProperty ( propertyHandles[p].property, snapshot, bottomSurface );

            if ( bottomPropertyValues == 0)
	    {
	       failed = true;
	       continue;
	    }

            topValues[p] = topPropertyValues->interpolate ( coordI, coordJ );

            if ( topValues[p] == topPropertyValues->getUndefinedValue ())
	    {
	       failed = true;
	       continue;
	    }

	    if (propertyHandles[p].operation & SaveTopDepth)
	    {
	       if (!foundSurfaceDepths[snapshot])
	       {
		  surfaceDepths[snapshot] = topValues[p];
		  foundSurfaceDepths[snapshot] = true;
	       }
	    }

            bottomValues[p] = bottomPropertyValues->interpolate ( coordI, coordJ );
            if ( bottomValues[p] == bottomPropertyValues->getUndefinedValue ())
	    {
	       failed = true;
	       continue;
	    }

	    if (propertyHandles[p].operation & CalcBelowMudLine)
	    {
	       assert (foundSurfaceDepths[snapshot]);
	       topValues[p] -= surfaceDepths[snapshot];
	       bottomValues[p] -= surfaceDepths[snapshot];
	    }

         }

	 if (failed) continue;

         outputFile << setw (colW - 1) << snapshot->getTime () << " ";
         for (p = 0; p < NumPropertyHandles; ++p)
         {
            outputFile << setw (colW - 1) << topValues[p] * propertyHandles[p].conversionFactor;
	    outputFile << " ";
            outputFile << setw (colW - 1) << bottomValues[p] * propertyHandles[p].conversionFactor;
	    if (p != NumPropertyHandles - 1) outputFile << " ";
         }
	 outputFile << endl;
      }
   }

   outputFile.close ();

   return true;
}

void showUsage (const char * message)
{
   cerr << endl;
   if (message)
   {
      cerr << argv0 << ": " << message << endl;
   }

   cerr << "Usage: " << argv0
      << " -project inputprojectfile [-save outputprojectfile]"
      << endl;
   exit (-1);
}

