#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dlfcn.h>

#include <assert.h>

#include <iostream>
#include <string>
using namespace std;

#include "database.h"
#include "cauldronschema.h"
#include "datatype.h"
#include "cauldronschemafuncs.h"

using database::Database;
using database::DataSchema;
using database::TableDefinition;
using database::Table;
using database::Record;
using database::Transaction;

#include "process.h"

static char * argv0 = 0;
static void showUsage (const char * message = 0);

string programVersion = IBSSHORTVERSION;

const int UndefinedTableValue = -9999;

bool verbose = false;
bool quiet = false;

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)

int main (int argc, char ** argv)
{
   char *inputFileName = "";
   char *outputFileName = "";
   char *outputDirectory = "";

   bool maps = false;
   bool inputonly = false;
   bool oneD = false;

   if ((argv0 = strrchr (argv[0], '/')) != 0)
      ++argv0;
   else
      argv0 = argv[0];

   for (int arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-save", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-save' is missing");
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
         inputFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-version", Max (5, strlen (argv[arg]))) == 0)
      {
         programVersion = argv[++arg];
      }
      else if (strncmp (argv[arg], "-verbose", Max (5, strlen (argv[arg]))) == 0)
      {
         verbose = true;
         quiet = false;
      }
      else if (strncmp (argv[arg], "-maps", Max (2, strlen (argv[arg]))) == 0)
      {
         maps = true;
      }
      else if (strncmp (argv[arg], "-inputonly", Max (2, strlen (argv[arg]))) == 0)
      {
         inputonly = true;
      }
      else if (strncmp (argv[arg], "-1d", Max (2, strlen (argv[arg]))) == 0)
      {
         oneD = true;
      }
      else if (strncmp (argv[arg], "-directory", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-directory' is missing");
            return -1;
         }
         outputDirectory = argv[++arg];
      }
      else if (strncmp (argv[arg], "-quiet", Max (5, strlen (argv[arg]))) == 0)
      {
         quiet = true;
         verbose = false;
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
   }

   if (programVersion[0] != 'v')
   {
      programVersion = "v" + programVersion;
   }

   if (inputFileName == "")
   {
      showUsage ("No project file specified");
   }

   if (outputDirectory == "")
   {
      showUsage ("No directory to output to specified");
   }

   if (outputFileName == "")
   {
      outputFileName = inputFileName;
   }

   mkdir (outputDirectory, 0777);

   struct stat buf;

   stat (outputDirectory, &buf);

   if (!S_ISDIR (buf.st_mode))
   {

      char errorstring[1024];

      sprintf (errorstring, "Could not create directory '%s'", outputDirectory);
      perror (errorstring);

      return 5;
   }

   DataSchema *cauldronSchema = database::createCauldronSchema ();
   TableDefinition *tableDef = cauldronSchema->getTableDefinition ("SourceRockLithoIoTbl");

   if (tableDef)
   {
      // Adding (volatile, won't be output) definition for obsoleted field UserDefined" << endl;
      tableDef->addFieldDefinition ("UserDefined", datatype::Int, "", "0");
   }

   Database *tables = Database::CreateFromFile (inputFileName, *cauldronSchema);

   delete cauldronSchema;

   if (!tables)
   {
      if (!quiet)
         cerr << "No tables were found in the project file" << endl;
      return 1;
   }

   // set the program version /////////////////////////////////////

   database::Table * projectIoTbl = tables->getTable ("ProjectIoTbl");
   if (!projectIoTbl)
   {
      if (!quiet)
         cerr << "No ProjectIoTbl table was found in the project file" << endl;
      return 2;
   }

   Record *projectIoRecord = projectIoTbl->getRecord (0);

   database::setProgramVersion (projectIoRecord, programVersion);

   // Inputonly ///////////////////////////////////////////////////

   if (inputonly)
   {
      Table *runStatusIoTbl = tables->getTable ("RunStatusIoTbl");

      assert (runStatusIoTbl);

      runStatusIoTbl->clear ();
      Record *runStatusIoRecord = runStatusIoTbl->createRecord ();

      setNrMCLoopsCompleted (runStatusIoRecord, 1);
      setMCCurrentSeedNumber (runStatusIoRecord, 367);
      setMCStatusOfLastRun (runStatusIoRecord, "Initial");
      setMCCalculationScope (runStatusIoRecord, "INITIAL");
      setOutputDirOfLastRun (runStatusIoRecord, "");
      setOutputDirCreatedBy (runStatusIoRecord, "");
      setRestartTempCalcTimeStep (runStatusIoRecord, 99999);
      setRestartPresCalcTimeStep (runStatusIoRecord, 99999);

      tables->clearTable ("TimeIoTbl");
      tables->clearTable ("DepthIoTbl");
      tables->clearTable ("TrapIoTbl");
      tables->clearTable ("MigrationIoTbl");
      tables->clearTable ("TrapperIoTbl");

      database::Table::iterator snapshotIoTblIter;

      database::Table * snapshotIoTbl = tables->getTable ("SnapshotIoTbl");
      if (!snapshotIoTbl)
         return 2;

      int snapshotIncrement = 1;
      int index = 0;

      for (snapshotIoTblIter = snapshotIoTbl->begin (); snapshotIoTblIter != snapshotIoTbl->end (); snapshotIoTblIter += snapshotIncrement)
      {
         snapshotIncrement = 1;
         if (getIsMinorSnapshot (*snapshotIoTblIter) == 1)
         {
            snapshotIoTblIter = snapshotIoTbl->removeRecord (snapshotIoTblIter);
            snapshotIncrement = 0;
            continue;
         }
         setSnapshotFileName (*snapshotIoTblIter, "");
      }
   }

   // 1D project related stuff ////////////////////////////////////

   string modellingMode = getModellingMode (projectIoRecord);

   if (modellingMode == "1d")
   {
      database::Table * wellLocIoTbl = tables->getTable ("WellLocIoTbl");
      if (!wellLocIoTbl)
      {
         if (!quiet)
            cerr << "No WellLocIoTbl table was found in the project file" << endl;
         return 2;
      }

      Record *wellLocIoRecord = wellLocIoTbl->getRecord (0);

      if (getXCoord (wellLocIoRecord) == 0)
         setXCoord (wellLocIoRecord, 0.01);
      if (getYCoord (wellLocIoRecord) == 0)
         setYCoord (wellLocIoRecord, 0.01);

      database::Table * projectIoTbl = tables->getTable ("ProjectIoTbl");
      if (!projectIoTbl)
      {
         if (!quiet)
            cerr << "No ProjectIoTbl table was found in the project file" << endl;
         return 2;
      }

      Record *projectIoRecord = projectIoTbl->getRecord (0);

      // may be set to -1 sometimes
      setWindowXMin (projectIoRecord, 0);
      setWindowXMax (projectIoRecord, 1);
      setWindowYMin (projectIoRecord, 0);
      setWindowYMax (projectIoRecord, 1);

   }

   // Update the source rocks /////////////////////////////////////
   database::Table * stratIoTbl = tables->getTable ("StratIoTbl");
   if (!stratIoTbl)
   {
      if (!quiet)
         cerr << "No StratIoTbl table was found in the project file" << endl;
      return 2;
   }

   database::Table::iterator stratIoTblIter;
   for (stratIoTblIter = stratIoTbl->begin (); stratIoTblIter != stratIoTbl->end (); ++stratIoTblIter)
   {
      Record *stratRecord = *stratIoTblIter;

      if (getChemicalCompaction (stratRecord) == UndefinedTableValue)
      {
         setChemicalCompaction (stratRecord, 0);
      }
   }

   database::Table * sourceRockLithoIoTbl = tables->getTable ("SourceRockLithoIoTbl");
   if (!sourceRockLithoIoTbl)
      return 2;

   database::Table::iterator sourceRockLithoIoTblIter;

   int sourceRockIncrement = 1;
   int index = 0;

   for (sourceRockLithoIoTblIter = sourceRockLithoIoTbl->begin ();
        sourceRockLithoIoTblIter != sourceRockLithoIoTbl->end (); sourceRockLithoIoTblIter += sourceRockIncrement)
   {
      sourceRockIncrement = 1;
      Record *sourceRockLithoRecord = *sourceRockLithoIoTblIter;
      Record *stratRecord = stratIoTbl->findRecord ("LayerName", database::getLayerName (sourceRockLithoRecord));

      if (stratRecord == 0)
      {
         if (!quiet)
         {
            cerr << inputFileName << ":" << endl;
            cerr << "  Removing sourceRockLithoIoRecord for non-existant formation \"" << getLayerName (sourceRockLithoRecord) << "\"" << endl << endl;
         }
         sourceRockLithoIoTblIter = sourceRockLithoIoTbl->removeRecord (sourceRockLithoIoTblIter);
         sourceRockIncrement = 0;
         continue;
      }

      ++index;

      if (getBaseSourceRockType (sourceRockLithoRecord) == "")
      {
         setBaseSourceRockType (sourceRockLithoRecord, getSourceRockType (sourceRockLithoRecord));
      }

      string sourceRockTypeOld = getSourceRockType (sourceRockLithoRecord);
      string baseSourceRockTypeOld = getBaseSourceRockType (sourceRockLithoRecord);

      string sourceRockTypeNew = sourceRockTypeOld;

      bool unknownSourceRockType = false;

      if (sourceRockTypeOld == "LacustrineAlgal")
         sourceRockTypeNew = "Type_I_CenoMesozoic_Lacustrine_kin";
      else if (sourceRockTypeOld == "MesozoicMarineShale")
         sourceRockTypeNew = "Type_II_Mesozoic_MarineShale_kin";
      else if (sourceRockTypeOld == "PaleozoicMarineShale")
         sourceRockTypeNew = "Type_II_Paleozoic_MarineShale_kin";
      else if (sourceRockTypeOld == "MesozoicCalcareousShale")
         sourceRockTypeNew = "Type_II_Mesozoic_Marl_kin";
      else if (sourceRockTypeOld == "VitriniticCoals")
         sourceRockTypeNew = "Type_III_MesoPaleozoic_VitriniticCoal_kin";

      else if (baseSourceRockTypeOld == "Type_I_CenoMesozoic_Lacustrine_kin");
      else if (baseSourceRockTypeOld == "Type_I_II_Mesozoic_MarineShale_lit");
      else if (baseSourceRockTypeOld == "Type_I_II_Cenozoic_Marl_kin");
      else if (baseSourceRockTypeOld == "Type_II_Mesozoic_MarineShale_kin");
      else if (baseSourceRockTypeOld == "Type_II_Paleozoic_MarineShale_kin");
      else if (baseSourceRockTypeOld == "Type_II_Mesozoic_Marl_kin");
      else if (baseSourceRockTypeOld == "Type_III_II_Mesozoic_HumicCoal_lit");
      else if (baseSourceRockTypeOld == "Type_III_MesoPaleozoic_VitriniticCoal_kin");

      else if (baseSourceRockTypeOld == "Type_I_CenoMesozoic_Lacustrine_kin_s");
      else if (baseSourceRockTypeOld == "Type_I_II_Mesozoic_MarineShale_lit_s");
      else if (baseSourceRockTypeOld == "Type_I_II_Cenozoic_Marl_kin_s");
      else if (baseSourceRockTypeOld == "Type_II_Mesozoic_MarineShale_kin_s");
      else if (baseSourceRockTypeOld == "Type_II_Paleozoic_MarineShale_kin_s");
      else if (baseSourceRockTypeOld == "Type_II_Mesozoic_Marl_kin_s");
      else if (baseSourceRockTypeOld == "Type_III_II_Mesozoic_HumicCoal_lit_s");
      else
      {
         unknownSourceRockType = true;
         if (!quiet)
         {
            cerr << inputFileName << ":" << endl;
	    if (sourceRockTypeOld == "")
	    {
	       cerr << "  Found unnamed source rock type of formation " << getLayerName (sourceRockLithoRecord) << endl << endl;
	    }
	    else
	    {
	       cerr << "  Did not recognize source rock type " << sourceRockTypeOld  << " of formation " << getLayerName (sourceRockLithoRecord) << endl << endl;
	    }
            sourceRockTypeNew = "Type_II_Paleozoic_MarineShale_kin";
         }
      }

      if (sourceRockTypeOld != sourceRockTypeNew)
      {
         if (!quiet)
         {
            cerr << inputFileName << ":" << endl;
	    if (sourceRockTypeOld == "")
	    {
	       cerr << "  Mapping unnamed source rock type of formation " << getLayerName (sourceRockLithoRecord) <<  " onto" << endl;
	    }
	    else
	    {
	       cerr << "  Mapping old-fashionedly named source rock type " << sourceRockTypeOld << " of formation " << getLayerName (sourceRockLithoRecord) << " onto" << endl;
	    }

            cerr << "  source rock type " << sourceRockTypeNew << endl;
            cerr << "  Please verify that this is correct" << endl << endl;
         }
         setBaseSourceRockType (sourceRockLithoRecord, sourceRockTypeNew);

         int userDefined = sourceRockLithoRecord->getValue < int >("UserDefined");

         if (userDefined || unknownSourceRockType)
         {
            char newName[128];

            sprintf (newName, "DerivedType_%d", index);
            sourceRockTypeNew = newName;
            if (!quiet)
            {
               cerr << inputFileName << ":" << endl;
               cerr << "  Assigning name \"" << sourceRockTypeNew << "\" to user defined source rock type with base type " << getBaseSourceRockType (sourceRockLithoRecord) << " of formation \"" <<
                     getLayerName (sourceRockLithoRecord) << "\"" << endl << endl;
            }
         }

         setSourceRockType (sourceRockLithoRecord, sourceRockTypeNew);
      }

      if (getSourceRockType1 (stratRecord) == "")
      {
         setSourceRockType1 (stratRecord, getSourceRockType (sourceRockLithoRecord));
      }
   }

   // Update the fluid types /////////////////////////////////////
   const double salinity_0 = 0; // ppm
   const double salinity_13 = 0.13; // ppm
   const double salinity_22 = 0.22; // ppm

#if 0
   const double liquidusTemperature_0 = 0; // degC
   const double liquidusTemperature_13 = -9.03; // degC
   const double liquidusTemperature_22 = -19.19; // degC
   const double solidusTemperatureOffset = -2.146; // degC
#endif

   database::Table * fluidTypeIoTbl = tables->getTable ("FluidtypeIoTbl");
   if (!fluidTypeIoTbl)
   {
      if (!quiet)
         cerr << "No FluidtypeIoTbl table was found in the project file" << endl;
      return 2;
   }

#if 0
   database::Table::iterator fluidTypeIoTblIter;

   for (fluidTypeIoTblIter = fluidTypeIoTbl->begin (); fluidTypeIoTblIter != fluidTypeIoTbl->end (); ++fluidTypeIoTblIter)
   {
      Record *fluidTypeRecord = *fluidTypeIoTblIter;
      string fluidType = getFluidtype (fluidTypeRecord);

      if (fluidType == "Std. Water") // salinity = 0 ppm
      {
         setLiquidusTemperature (fluidTypeRecord, liquidusTemperature_0);
         setSolidusTemperature (fluidTypeRecord, liquidusTemperature_0 + solidusTemperatureOffset);
      }
      else if (fluidType == "Std. Marine Water") // salinity = 0.13 ppm
      {
         setLiquidusTemperature (fluidTypeRecord, liquidusTemperature_13);
         setSolidusTemperature (fluidTypeRecord, liquidusTemperature_13 + solidusTemperatureOffset);
      }
      else if (fluidType == "Std. Hyper Saline Water") // salinity = 0.22 ppm
      {
         setLiquidusTemperature (fluidTypeRecord, liquidusTemperature_22);
         setSolidusTemperature (fluidTypeRecord, liquidusTemperature_22 + solidusTemperatureOffset);
      }
      else
      {
         double salinity = getSalinity (fluidTypeRecord);
         double liquidusTemperature = 0;

         if (salinity < salinity_13)
         {
            liquidusTemperature = salinity / salinity_13 * liquidusTemperature_13;
         }
         else if (salinity < salinity_22)
         {
            liquidusTemperature = ((salinity - salinity_13) / (salinity_22 - salinity_13)) *
                  (liquidusTemperature_22 - liquidusTemperature_13) + liquidusTemperature_13;
         }
         else
         {
            liquidusTemperature = liquidusTemperature_22;
         }

         setLiquidusTemperature (fluidTypeRecord, liquidusTemperature);
         setSolidusTemperature (fluidTypeRecord, liquidusTemperature + solidusTemperatureOffset);

         if (!quiet)
         {
            cerr << inputFileName << ":" << endl;
            cerr << "  Setting LiquidusTemperature for fluid type \"" << fluidType << "\" to " << liquidusTemperature << endl;
            cerr << "  Setting SolidusTemperature for fluid type \"" << fluidType << "\" to " << liquidusTemperature + solidusTemperatureOffset << endl;
            cerr << "  Please verify that these temperatures are correct" << endl << endl;
         }
      }
   }
#endif

   // HDF5 output ///////////////////////////////////////////////////

   Table *ioOptionsIoTbl = tables->getTable ("IoOptionsIoTbl");

   assert (ioOptionsIoTbl);

   Record *ioOptionsIoRecord = ioOptionsIoTbl->getRecord (0);

   setMapType (ioOptionsIoRecord, "HDF5");

   Table *gridMapTbl = tables->getTable ("GridMapIoTbl");

   if (gridMapTbl)
   {
      vector < string > fileNames;
      vector < string >::iterator fileNamesIter;

      map < string, string > fileNameMap;

      for (int i = 0; i < gridMapTbl->size (); ++i)
      {
         Record *gridMapIoRecord = gridMapTbl->getRecord (i);

         assert (gridMapIoRecord);
         string mapName = getMapName (gridMapIoRecord);
         string mapType = getMapType (gridMapIoRecord);
         string mapFileName = getMapFileName (gridMapIoRecord);
         int mapSeqNbr = getMapSeqNbr (gridMapIoRecord);

         bool fileNameFound = false;

         for (fileNamesIter = fileNames.begin (); fileNamesIter != fileNames.end (); ++fileNamesIter)
         {
            if (*fileNamesIter == mapFileName)
            {
               fileNameFound = true;
            }
         }

         if (!fileNameFound)
            fileNames.push_back (mapFileName);

         char *args[20];
         int n = 0;

         // These are the map types grid2grid can handle.
         if (maps && (mapType == "DECBINARY" || mapType == "DECASCII" || mapType == "ZYCOR" || mapType == "CPS3" ||
                      mapType == "EPIRUS" || mapType == "LANDMARK" || mapType == "CHARISMA" || mapType == "XYZ" || mapType == "HDF5"))
         {

            string outputMapFileName;

            outputMapFileName = mapFileName;

            string::size_type dotPos = outputMapFileName.rfind ('.');
            if (dotPos != string::npos)
            {
               outputMapFileName.erase (dotPos, string::npos);
            }
            outputMapFileName += ".HDF";


            setMapFileName (gridMapIoRecord, outputMapFileName);
            setMapType (gridMapIoRecord, "HDF5");

            setFileId (gridMapIoRecord, -1);
            setHDF5FileName (gridMapIoRecord, "");

            if (fileNameFound)
               continue;

	    string fullOutputMapFileName = outputDirectory;
	    fullOutputMapFileName += "/";
	    fullOutputMapFileName += outputMapFileName;
	    
            args[n] = strdup ("grid2grid");
            n++;

            char *ibsVersion = getenv ("IBS_VERSION");

            if (ibsVersion && strlen (ibsVersion) > 1)
            {
               char versionArg[32];

               strcpy (versionArg, "-");
               strcat (versionArg, ibsVersion);
               args[n] = versionArg;
               n++;
            }

            args[n] = strdup ("-input");
            n++;
            args[n] = strdup (mapFileName.c_str ());
            n++;
            args[n] = strdup (mapType.c_str ());
            n++;

            args[n] = strdup ("-output");
            n++;
            args[n] = strdup (fullOutputMapFileName.c_str ());
            n++;
            args[n] = strdup ("HDF5");
            n++;

            if (mapType == "XYZ")
            {
               char originXStr[32];
               char originYStr[32];
               char deltaXStr[32];
               char deltaYStr[32];
               char resolutionXStr[32];
               char resolutionYStr[32];

               sprintf (originXStr, "%lf", getXCoord (projectIoRecord));
               sprintf (originYStr, "%lf", getYCoord (projectIoRecord));
               sprintf (deltaXStr, "%lf", getDeltaX (projectIoRecord));
               sprintf (deltaYStr, "%lf", getDeltaY (projectIoRecord));
               sprintf (resolutionXStr, "%d", getNumberX (projectIoRecord));
               sprintf (resolutionYStr, "%d", getNumberY (projectIoRecord));

               args[n] = strdup ("-origin");
               n++;
               args[n] = originXStr;
               n++;
               args[n] = originYStr;
               n++;
               args[n] = strdup ("-resolution");
               n++;
               args[n] = resolutionXStr;
               n++;
               args[n] = resolutionYStr;
               n++;
               args[n] = strdup ("-delta");
               n++;
               args[n] = deltaXStr;
               n++;
               args[n] = deltaYStr;
               n++;
            }
            args[n] = 0;


	    if (!quiet)
	    {
	       cerr << inputFileName << ":" << endl;
	       cerr << "  Converting " << mapFileName << " to HDF5 format" << endl << endl;
	    }

            processCmd ("grid2grid", args, true);
         }
         else
         {
            if (fileNameFound)
               continue;

            args[n] = strdup ("/bin/cp");
            n++;
            args[n] = strdup (mapFileName.c_str ());
            n++;
            args[n] = strdup (outputDirectory);
            n++;
            args[n] = 0;

            if (!quiet)
            {
               cout << inputFileName << ":" << endl;
               cout << "  Copying map file " << mapFileName.c_str () << endl << endl;
            }
            processCmd ("/bin/cp", args, true);
         }
      }
   }

   // Convert 1D projects
   database::Table::iterator relatedProjectsIoTblIter;

   database::Table * relatedProjectsIoTbl = tables->getTable ("RelatedProjectsIoTbl");
   if (!relatedProjectsIoTbl)
      return 2;

   for (relatedProjectsIoTblIter = relatedProjectsIoTbl->begin (); relatedProjectsIoTblIter != relatedProjectsIoTbl->end (); ++relatedProjectsIoTblIter)
   {
      Record *relatedProjectsIoRecord = *relatedProjectsIoTblIter;

      char *args[20];
      int n = 0;

      if (oneD)
      {
         args[n] = strdup ("upcauldron");
         n++;

         char *ibsVersion = getenv ("IBS_VERSION");

         if (ibsVersion && strlen (ibsVersion) > 1)
         {
            char versionArg[32];

            strcpy (versionArg, "-");
            strcat (versionArg, ibsVersion);
            args[n] = versionArg;
            n++;
         }

	 if (inputonly)
	 {
	    args[n] = strdup ("-inputonly");
	    n++;
	 }

         args[n] = strdup ("-project");
         n++;

         args[n] = strdup (getFilename (relatedProjectsIoRecord).c_str ());
         n++;

         args[n] = strdup ("-directory");
         n++;
         args[n] = strdup (outputDirectory);
         n++;
	 args[n] = 0;

         if (!quiet)
         {
            cout << inputFileName << ":" << endl;
            cout << "  Converting 1D project file " << getFilename (relatedProjectsIoRecord).c_str () << endl << endl;
         }
         processCmd ("upcauldron", args, true);
      }
      else
      {
         args[n] = strdup ("/bin/cp");
         n++;
         args[n] = strdup (getFilename (relatedProjectsIoRecord).c_str ());
         n++;
         args[n] = strdup (outputDirectory);
         n++;
	 args[n] = 0;

         if (!quiet)
         {
            cout << inputFileName << ":" << endl;
            cout << "  Copying 1D project file " << getFilename (relatedProjectsIoRecord).c_str () << endl << endl;
         }
         processCmd ("/bin/cp", args, true);

      }
   }

   char fullpath[1024];

   sprintf (fullpath, "%s/%s", outputDirectory, outputFileName);
   tables->saveToFile (fullpath);

   waitForChildren ();

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
         << "                  -directory <directory-path>" << endl
         << "                  [-output <cauldron-project-file>]" << endl
         << "                  [-version <programVersion>]" << endl
         << "                  [-inputonly]" << endl
         << "                  [-maps]" << endl
         << "                  [-1d]" << endl
         << "                  [-verbose]" << endl
         << "                  [-quiet]" << endl
         << "                  [-help]" << endl
         << "                  [-?]" << endl
         << "                  [-usage]" << endl
         << endl
         << "    -project           The cauldron project file." << endl
         << "    -directory         Create files in specified directory" << endl
         << "    -output            Output cauldron project file, defaults to the input file name." << endl
         << "    -version           The value of the ProgramVersion variable, probably obsolete." << endl
         << "    -inputonly         Clean simulation output tables" << endl
         << "    -maps              Convert non-HDF5 map files to HDF5 map files." << endl
         << "    -1d                Convert related 1D project files as well" << endl
         << "    -verbose           Generate some extra output, nothing at the moment." << endl
         << "    -quiet             Do not generate any output." << endl
         << "    -help              Print this message." << endl
         << "    -?                 Print this message." << endl << "    -usage             Print this message." << endl << endl;
   exit (-1);
}

