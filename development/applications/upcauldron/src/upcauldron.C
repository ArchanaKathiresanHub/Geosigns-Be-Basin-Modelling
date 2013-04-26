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
   char * inputFileName = "";
   char * outputFileName = "";

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

   if (outputFileName == "")
   {
      outputFileName = inputFileName;
   }

   DataSchema * cauldronSchema = database::createCauldronSchema ();
   TableDefinition * tableDef = cauldronSchema->getTableDefinition("SourceRockLithoIoTbl");
   if (tableDef)
   {
      // Adding (volatile, won't be output) definition for obsoleted field UserDefined" << endl;
      tableDef->addFieldDefinition ("UserDefined", datatype::Int, "", "0");
   }

   Database * tables = Database::CreateFromFile (inputFileName, * cauldronSchema);
   delete cauldronSchema;

   if (!tables)
   {
      if (!quiet) cerr << "No tables were found in the project file" << endl;
      return 1;
   }

   // set the program version /////////////////////////////////////
   database::Table * projectIoTbl = tables->getTable ("ProjectIoTbl");
   if (!projectIoTbl)
   {
      if (!quiet) cerr << "No ProjectIoTbl table was found in the project file" << endl;
      return 2;
   }

   Record *projectIoRecord = projectIoTbl->getRecord (0);
   database::setProgramVersion (projectIoRecord, programVersion);

   // 1D project related stuff ////////////////////////////////////

   string modellingMode = getModellingMode (projectIoRecord);

   if (modellingMode == "1d")
   {
      database::Table * wellLocIoTbl = tables->getTable ("WellLocIoTbl");
      if (!wellLocIoTbl)
      {
	 if (!quiet) cerr << "No WellLocIoTbl table was found in the project file" << endl;
	 return 2;
      }

      Record *wellLocIoRecord = wellLocIoTbl->getRecord (0);

      if (getXCoord (wellLocIoRecord) == 0) setXCoord  (wellLocIoRecord, 0.01);
      if (getYCoord (wellLocIoRecord) == 0) setYCoord  (wellLocIoRecord, 0.01);
   }



   // Update the source rocks /////////////////////////////////////
   database::Table * stratIoTbl = tables->getTable ("StratIoTbl");
   if (!stratIoTbl)
   {
      if (!quiet) cerr << "No StratIoTbl table was found in the project file" << endl;
      return 2;
   }

   database::Table::iterator stratIoTblIter;
   for (stratIoTblIter = stratIoTbl->begin();
	 stratIoTblIter != stratIoTbl->end();
	 ++stratIoTblIter)
   {
      Record * stratRecord = * stratIoTblIter;
      if (getChemicalCompaction (stratRecord) == UndefinedTableValue)
      {
	 setChemicalCompaction (stratRecord, 0);
      }
   }

   database::Table * sourceRockLithoIoTbl = tables->getTable ("SourceRockLithoIoTbl");
   if (!sourceRockLithoIoTbl) return 2;

   database::Table::iterator sourceRockLithoIoTblIter;

   int increment = 1;
   int index = 0;
   for (sourceRockLithoIoTblIter = sourceRockLithoIoTbl->begin();
	 sourceRockLithoIoTblIter != sourceRockLithoIoTbl->end();
	 sourceRockLithoIoTblIter += increment)
   {
      increment = 1;
      Record * sourceRockLithoRecord = * sourceRockLithoIoTblIter;
      Record * stratRecord = stratIoTbl->findRecord("LayerName", database::getLayerName (sourceRockLithoRecord));

      if (stratRecord == 0) 
      {
	 if (!quiet) cerr << "- Removing sourceRockLithoIoRecord for non-existant formation \"" << getLayerName (sourceRockLithoRecord)  << "\"" << endl;
	 sourceRockLithoIoTblIter = sourceRockLithoIoTbl->removeRecord(sourceRockLithoIoTblIter);
	 increment = 0;
	 continue;
      }

      ++index;

      if (getBaseSourceRockType(sourceRockLithoRecord) == "")
      {
	 setBaseSourceRockType(sourceRockLithoRecord, getSourceRockType (sourceRockLithoRecord));
      }

      string sourceRockTypeOld = getSourceRockType (sourceRockLithoRecord);
      string baseSourceRockTypeOld = getBaseSourceRockType (sourceRockLithoRecord);

      string sourceRockTypeNew = sourceRockTypeOld;

      if (sourceRockTypeOld == "LacustrineAlgal") sourceRockTypeNew = "Type_I_CenoMesozoic_Lacustrine_kin";
      else if (sourceRockTypeOld == "MesozoicMarineShale") sourceRockTypeNew = "Type_II_Mesozoic_MarineShale_kin";
      else if (sourceRockTypeOld == "PaleozoicMarineShale") sourceRockTypeNew = "Type_II_Paleozoic_MarineShale_kin";
      else if (sourceRockTypeOld == "MesozoicCalcareousShale") sourceRockTypeNew = "Type_II_Mesozoic_Marl_kin";
      else if (sourceRockTypeOld == "VitriniticCoals") sourceRockTypeNew = "Type_III_MesoPaleozoic_VitriniticCoal_kin";

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
	 if (!quiet) cerr << "Did not recognize source rock type " << sourceRockTypeOld << ", aborting " << argv0 << endl;
	 return 4;
      }

      if (sourceRockTypeOld != sourceRockTypeNew)
      {
	 if (!quiet) cerr << "- Mapping old-fashionedly named source rock type " << sourceRockTypeOld << " onto" << endl;
	 if (!quiet) cerr << "  source rock type " << sourceRockTypeNew << endl;
	 if (!quiet) cerr << "  Please verify that this is correct" << endl;
	 setBaseSourceRockType(sourceRockLithoRecord, sourceRockTypeNew);

	 int userDefined = sourceRockLithoRecord->getValue<int> ("UserDefined");

	 if (userDefined)
	 {
	    char newName[128];
	    sprintf (newName, "DerivedType_%d", index);
	    sourceRockTypeNew = newName;
	    if (!quiet) cerr << "- Assigning name \"" << sourceRockTypeNew << "\" to user defined source rock type of formation \"" << getLayerName (sourceRockLithoRecord)  << "\"" << endl;
	 }

	 setSourceRockType(sourceRockLithoRecord, sourceRockTypeNew);
      }

      if (getSourceRockType1(stratRecord) == "")
      {
	 setSourceRockType1(stratRecord, getSourceRockType (sourceRockLithoRecord));
      }
   }
   
   // Update the fluid types /////////////////////////////////////
   const double salinity_0 = 0; // ppm
   const double salinity_13 = 0.13; // ppm
   const double salinity_22 = 0.22; // ppm

   const double liquidusTemperature_0 = 0; // degC
   const double liquidusTemperature_13 = -9.03; // degC
   const double liquidusTemperature_22 = -19.19; // degC
   const double solidusTemperatureOffset = -2.146; // degC

   database::Table * fluidTypeIoTbl = tables->getTable ("FluidtypeIoTbl");
   if (!fluidTypeIoTbl)
   {
      if (!quiet) cerr << "No FluidtypeIoTbl table was found in the project file" << endl;
      return 2;
   }

   database::Table::iterator fluidTypeIoTblIter;

   for (fluidTypeIoTblIter = fluidTypeIoTbl->begin();
	 fluidTypeIoTblIter != fluidTypeIoTbl->end();
	 ++fluidTypeIoTblIter)
   {
      Record * fluidTypeRecord = * fluidTypeIoTblIter;
      string fluidType = getFluidtype (fluidTypeRecord);

      if (fluidType == "Std. Water") // salinity = 0 ppm
      {
	 setLiquidusTemperature (fluidTypeRecord, liquidusTemperature_0 );
	 setSolidusTemperature (fluidTypeRecord, liquidusTemperature_0 + solidusTemperatureOffset );
      }
      else if (fluidType == "Std. Marine Water") // salinity = 0.13 ppm
      {
	 setLiquidusTemperature (fluidTypeRecord, liquidusTemperature_13 );
	 setSolidusTemperature (fluidTypeRecord, liquidusTemperature_13 + solidusTemperatureOffset );
      }
      else if (fluidType == "Std. Hyper Saline Water") // salinity = 0.22 ppm
      {
	 setLiquidusTemperature (fluidTypeRecord, liquidusTemperature_22 );
	 setSolidusTemperature (fluidTypeRecord, liquidusTemperature_22 + solidusTemperatureOffset );
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

	 setLiquidusTemperature (fluidTypeRecord, liquidusTemperature );
	 setSolidusTemperature (fluidTypeRecord, liquidusTemperature + solidusTemperatureOffset );

	 if (!quiet) cerr << "- Setting LiquidusTemperature for fluid type \"" << fluidType << "\" to " << liquidusTemperature << endl;
	 if (!quiet) cerr << "  Setting SolidusTemperature for fluid type \"" << fluidType << "\" to " << liquidusTemperature + solidusTemperatureOffset << endl;
	 if (!quiet) cerr << "  Please verify that these temperatures are correct" << endl;
      }
   }

   tables->saveToFile(outputFileName);
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
         << "                  [-output <cauldron-project-file>]" << endl
         << "                  [-version programVersion]" << endl
         << "                  [-verbose]" << endl
         << "                  [-quiet]" << endl
         << "                  [-help]" << endl
         << "                  [-?]" << endl
         << "                  [-usage]" << endl
         << endl
         << "    -project           The cauldron project file." << endl
         << "    -output            Output cauldron project file, defaults to the input file name." << endl
         << "    -version           The value of the ProgramVersion variable, probably obsolete." << endl
         << "    -verbose           Generate some extra output, nothing at the moment." << endl
         << "    -quiet             Do not generate any output." << endl
         << "    -help              Print this message." << endl
         << "    -?                 Print this message." << endl
         << "    -usage             Print this message." << endl << endl;
   exit (-1);
}

