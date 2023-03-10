//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <map>
#include <values.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// DataAccess API includes
#include "GridMap.h"
#include "Grid.h"
#include "ProjectHandle.h"
#include "Snapshot.h"
#include "Surface.h"
#include "Formation.h"
#include "Reservoir.h"
#include "LithoType.h"
#include "Property.h"
#include "PropertyValue.h"

#include <string.h>


using namespace DataAccess;
using namespace Interface;

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)

// Maximum count of names in slnspec file
#define MNC	99

typedef double (* ConversionFunc) (double);
typedef vector<const GridMap *> GridMapVector;
typedef vector<const Formation *> FormationVector;
typedef std::map<string, int> LithologyId;
LithologyId litholog_id;

const double CauldronUndefined = 99999;
const double EclipseUndefined = -9999;
const int NumCorners = 8;
const int CornerOffsets[NumCorners][3] = { {0, 0, -1}, {1, 0, -1}, {1, 1, -1}, {0, 1, -1}, {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0} };

static char * argv0 = 0;
static void showUsage (const char * message = 0);

bool verbose = false;
bool debug = false;

bool doAscii = true;
bool doBinary = true;
FILE *fpw = NULL;

struct PropertyConversion
{
   const string cauldronName;
   const string eclipseName;
   ConversionFunc converter;
   const int idCellType;
   const bool hasBasement;
};

   template <class T>
T swapBytes (T value)
{
   union VC {
      T value;
      char c[sizeof (T)];
   };
   VC in, out;
   in.value = value;
   for (int i = 0; i < sizeof (T); ++i)
   {
      out.c[i] = in.c[sizeof (T) - i - 1];
   }

   return out.value;
}

   template <class T>
void writeValue (T value, bool swapMode)
{
   if (!swapMode)
   {
      fwrite (&value, 1, sizeof (T), fpw);
   }
   else
   {
      T swappedValue = swapBytes (value);
      fwrite (&swappedValue, 1, sizeof (T), fpw);
   }
}

static double PascalToMegaPascal (double in)
{
   return 1e-6 * in;
}

static double CelsiusToKelvin (double in)
{
   return in + 273.15;
}

static double PercentageToFraction (double in)
{
   return in / 100;
}

static double Negate (double in)
{
   return -in;
} 

int findLithologId (double density, double heatprod, double porosurf );
void writeEGRIDHeader( const bool& swapMode, const int& numI, const int& numJ,
      const double& f_posX,  const double& f_posY, const int& numberOfHorizons );
void writeINITHeader( const bool& swapMode, const int& numI, const int& numJ, const int& numberOfHorizons );
void checkDataBlocks (void);
void writeDelimiter ();
void writeKeywordRecord (char * keyword, int numberOfElements, char * elementFormat);
void startDataBlocks (int numItems, int size = 4);
void writeWord (char * word);

// gets a gridmap value of a point within a cell, even if one of the corners is undefined
double GetValue (const GridMap * gridMap, double i, double j, double k);

void GetCornerIndices (double kIndices[], int k, int numK);

/// Conversions between Cauldron properties and Eclipse properties
PropertyConversion conversions[] =
{
   { "Depth",                  "DEPTH   ", Negate, 0, true },
   { "Temperature",            "TEMP    ", CelsiusToKelvin, 0, true },
   { "HydroStaticPressure",    "HSPRES  ", 0, 0, false },
   { "LithoStaticPressure",    "LSPRES  ", 0, 0, true },
   { "Pressure",               "PRESSURE", 0, 0, false },
   { "Velocity",               "SONICVEL", 0, 1, true },
   { "ThCond",                 "TCOND   ", 0, 1, true },
   { "Reflectivity",           "REFLECTI", 0, 1, true },
   { "OverPressure",           "OVERPRES", 0, 0, false },
   { "Ves",                    "VES     ", PascalToMegaPascal, 0, false },
   { "MaxVes",                 "MAXVES  ", PascalToMegaPascal, 0, false },
   { "HorizontalPermeability", "PERMH   ", 0, 1, false },
   { "Permeability",           "PERMV   ", 0, 1, false },
   { "Vr",                     "VR      ", 0, 0, false },
   { "HeatFlowZ",              "VHFLOW  ", 0, 1, true },
   { "HeatFlowY",              "HFLOWY  ", 0, 1, true },
   { "HeatFlowX",              "HFLOWX  ", 0, 1, true },
   { "FluidVelocityX",         "FVELX   ", 0, 1, false },
   { "FluidVelocityY",         "FVELY   ", 0, 1, false },
   { "FluidVelocityZ",         "FVELZ   ", Negate, 1, false },
   { "Porosity",               "POROSITY", PercentageToFraction, 1, false },
   { "Diffusivity",            "DIFFUSIV", 0, 1, true },
   { "BulkDensity",            "BULKDENS", 0, 1, true },
   { "", "", 0, 0, false }
};

int main (int argc, char ** argv)
{
   string projectFileName;
   string outputFileName;
   char outputFileNameBin[256] = "";
   char uWord[9];
   int nlen, dNum, arg;
   float fNum;

   if ((argv0 = strrchr (argv[0], '/')) != 0)
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }

   double minPosX = 1e6, minPosY = 1e6;
   double maxPosX = -1e6, maxPosY = -1e6;
   double minDepth = 1e6;
   double maxDepth = -1e6;
   double snapshotTime = 0;
   bool basement = false;

   litholog_id["Std. Sandstone"] = 0;
   litholog_id["SM. Sandstone"] = 0;
   litholog_id["Std. Shale"] = 10;
   litholog_id["SM.Mudst.40%Clay"] = 10;
   litholog_id["SM.Mudst.50%Clay"] = 10;
   litholog_id["SM.Mudst.60%Clay"] = 10;
   litholog_id["Std. Siltstone"] = 1;
   litholog_id["Std.Grainstone"] = 17;
   litholog_id["Std.Dolo.Grainstone"] = 17;
   litholog_id["Std.Lime Mudstone"] = 8;
   litholog_id["Std.Dolo.Mudstone"] = 17;
   litholog_id["Std. Chalk"] = 7;
   litholog_id["Std. Marl"] = 7;
   litholog_id["Std. Anhydrite"] = 13;
   litholog_id["Std. Salt"] = 12;
   litholog_id["Sylvite"] = 12;
   litholog_id["Std. Coal"] = 14;
   litholog_id["Std. Basalt"] = 16;
   litholog_id["Crust"] = 16;
   litholog_id["Litho. Mantle"] = 16;
   litholog_id["Astheno. Mantle"] = 16;
   litholog_id["HEAT Sandstone"] = 0;
   litholog_id["HEAT Shale"] = 10;
   litholog_id["HEAT Limestone"] = 7;
   litholog_id["HEAT Dolostone"] = 17;
   litholog_id["HEAT Chalk"] = 7;

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
      else if (strncmp (argv[arg], "-snapshot", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-snapshot' is missing");
            return -1;
         }
         snapshotTime = atof (argv[++arg]);
      }
      else if (strncmp (argv[arg], "-basement", Max (3, strlen (argv[arg]))) == 0)
      {
         basement = true;
      }
      else if (strncmp (argv[arg], "-debug", Max (2, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if (strncmp (argv[arg], "-asciionly", Max (2, strlen (argv[arg]))) == 0)
      {
         if (!doAscii)          // turned off by '-binaryonly'
         {
            showUsage ("Only use one of '-asciionly' and '-binaryonly'");
            return -1;
         }
         else
         {
            doBinary = false;
         }
      }
      else if (strncmp (argv[arg], "-binaryonly", Max (3, strlen (argv[arg]))) == 0)
      {
         if (!doBinary)         // turned off by '-asciionly'
         {
            showUsage ("Only use one of '-asciionly' and '-binaryonly'");
            return -1;
         }
         else
         {
            doAscii = false;
         }
      }
      else if (strncmp (argv[arg], "-verbose", Max (2, strlen (argv[arg]))) == 0)
      {
         verbose = true;
      }
      else if (strncmp (argv[arg], "-", 1) != 0)
      {
         if (projectFileName == "")
            projectFileName = argv[arg];
         else
            outputFileName = argv[arg];
      }
      else
      {
         showUsage ();
         return -1;
      }
   }

   if (projectFileName == "")
   {
      showUsage ("No project file specified");
      return -1;
   }

   ProjectHandle *projectHandle = OpenCauldronProject (projectFileName, "r");

   if (projectFileName.length () == 0)
   {
      cerr << "Could not open project file " << projectFileName << endl;
      return -1;
   }

   size_t dotPos = projectFileName.find (".project");
   ofstream outputFile;

   if (outputFileName.length () == 0)
   {
      outputFileName = projectFileName.substr (0, dotPos);
   }

   if (doAscii)
   {
      outputFile.open ((outputFileName + ".ECL").c_str ());

      if (outputFile.fail ())
         return -1;
   }

   // open file for binary stream
   if (doBinary)
   {
      sprintf (outputFileNameBin, "%s.EGRID", outputFileName.c_str ());
      fpw = fopen (outputFileNameBin, "wb");
   }

   bool swapMode = true;

   const Snapshot *snapshot = projectHandle->findSnapshot (snapshotTime);

   if (!snapshot)
   {
      cerr << "No calculations have been made for snapshot time " << snapshotTime << endl;
      return -1;
   }

   if (verbose)
   {
      cout << "Using snapshot " << setprecision (10) << snapshot->getTime () << endl;
   }

   const Property *depthProperty = projectHandle->findProperty ("Depth");

   if (!depthProperty)
   {
      cerr << "Could not find the Depth property in the project file " << endl << "Are you sure the project file contains output data?" << endl;
      return -1;
   }

   // Get the depth volumes for all formations to build the eclipse grid.
   PropertyValueList *depthPropertyValueList = projectHandle->getPropertyValues (FORMATION, depthProperty, snapshot, 0, 0, 0);

   if (depthPropertyValueList->size () == 0)
   {
      cerr << "Could not find the Depth property results in the project file " << endl << "Are you sure the project file contains output data?" << endl;
      return -1;
   }
   
   // number of horizons
   unsigned int numberOfHorizons = 0;

   const Grid *grid = projectHandle->getLowResolutionOutputGrid ();
   const GridMap *gridMap = 0;

   const GridMap *topGridMap = 0;
   const GridMap *bottomGridMap = 0;
   const PropertyValue *propertyValue;

   PropertyValueList::iterator propertyValueIter;
   if (debug)
   {
      cerr << "Formations with Depth property found: " << endl;
   }

   if (verbose)
      cout << "Writing COORD" << endl;

   for (propertyValueIter = depthPropertyValueList->begin (); propertyValueIter != depthPropertyValueList->end (); ++propertyValueIter)
   {
      propertyValue = *propertyValueIter;

      if (propertyValue->getFormation () == 0 || (!basement && propertyValue->getFormation ()->kind () == BASEMENT_FORMATION))
      {
         continue;
      }

      if (debug)
      {
         propertyValue = *propertyValueIter;
         const Formation *formation = propertyValue->getFormation ();

         assert (formation);
         formation->printOn (cerr);
      }

      if (verbose)
      {
         cout << '-';
         cout.flush ();
      }

      gridMap = propertyValue->getGridMap ();
      assert (gridMap != 0);

      if (!topGridMap)
      {
         topGridMap = gridMap;
      }

      numberOfHorizons += gridMap->getDepth ();
   }

   if (verbose)
      cout << endl;

   bottomGridMap = gridMap;

   numberOfHorizons++;

   if (debug)
   {
      cerr << "Grid = (" << grid->numI () + 1 << ", " << grid->numJ () + 1 << ", " << numberOfHorizons << ")" << endl;
   }

   if (doAscii)
   {
      outputFile << "GRIDUNIT" << endl << "'METRES'" << endl << "/" << endl << endl;
   }

   unsigned int numI = grid->numI ();
   unsigned int numJ = grid->numJ ();
   unsigned int bottomIndex = bottomGridMap->getDepth () - 1;

   //------------- Binary data
   if (doBinary)
   {
      double f_posX, f_posY;

      grid->getPosition ((unsigned int) 0, (unsigned int) (numJ - 1), f_posX, f_posY);
      writeEGRIDHeader (swapMode, numI + 1, numJ + 1, f_posX, f_posY, numberOfHorizons);
   }
   //-------------

   if (doAscii)
   {
      outputFile << "SPECGRID" << endl << "    " << numI << "  " << numJ << "  " << numberOfHorizons - 1 << "  1  F /" << endl << endl;

      outputFile << "COORD " << endl;
   }

   unsigned int i, j, k;
   double ii, jj;
   double kk;

   for (j = 0; j <= numJ; ++j)
   {
      // eclipse node coordinates are cauldron cell values, except at the edges
      if (j == 0) jj = j;
      else if (j == numJ) jj = j - 1;
      else jj = j - 0.5;

      for (i = 0; i <= numI; ++i)
      {
	 // eclipse node coordinates are cauldron cell coordinates, except at the edges
	 if (i == 0) ii = i;
	 else if (i == numI) ii = i - 1;
	 else ii = i - 0.5;

         double posI, posJ;
         double topDepth;
         double bottomDepth;

         bool result = grid->getPosition (ii, jj, posI, posJ);

         assert (result);

         minPosX = Min (minPosX, posI);
         maxPosX = Max (maxPosX, posI);

         minPosY = Min (minPosY, posJ);
         maxPosY = Max (maxPosY, posJ);

	 // get values even if one of or more of the cauldron node values are undefined
         topDepth = GetValue (topGridMap, ii, jj, (double) 0);
         bottomDepth = GetValue (bottomGridMap, ii, jj, (double) bottomIndex);

	 topDepth = -topDepth;
	 minDepth = Min (minDepth, topDepth);

	 bottomDepth = -bottomDepth;
	 maxDepth = Max (maxDepth, bottomDepth);

         if (doAscii)
         {
            outputFile << posI << " " << posJ << " " << (std::isnan (bottomDepth) ? EclipseUndefined : bottomDepth) << endl;
            outputFile << "  " << posI << " " << posJ << " " << (std::isnan( topDepth ) ? EclipseUndefined : topDepth) << endl;
         }

         //------------- Binary data
         if (doBinary)
         {
            double values[6] = { posI, posJ, bottomDepth, posI, posJ, topDepth };

            for (int index = 0; index < 6; ++index)
            {
               fNum = (float) values[index];
               writeValue (fNum, swapMode);
               checkDataBlocks ();
            }
         }
         //------------- 
      }
   }
   if (doAscii)
   {
      outputFile << "/" << endl << endl;

      outputFile << "ZCORN" << endl;
   }

   int noItems = -1;

   //------------- Binary data
   if (doBinary)
   {
      writeDelimiter ();
      noItems = 2 * numI * 2 * numJ * 2 * (numberOfHorizons - 1);
      writeKeywordRecord ("ZCORN", noItems, "REAL");

      writeDelimiter ();
      startDataBlocks (noItems);
   }
   //--------------

   if (verbose)
      cout << "Writing ZCORN" << endl;

   unsigned int numDepth = 0;

   int itemCount = 0;
   int totalK = 0;

   for (propertyValueIter = depthPropertyValueList->begin (); propertyValueIter != depthPropertyValueList->end (); ++propertyValueIter)
   {
      const PropertyValue *propertyValue = *propertyValueIter;

      if (propertyValue->getFormation () == 0 || (!basement && propertyValue->getFormation ()->kind () == BASEMENT_FORMATION))
      {
         continue;
      }

      gridMap = propertyValue->getGridMap ();
      int numK = gridMap->getDepth ();

      totalK += numK;

      if (verbose)
      {
         cout << '-';
         cout.flush ();
      }

      for (k = 0; k < numK; ++k)
      {
	 double kIndices[2];
	 GetCornerIndices (kIndices, k, numK);

	 for (int kx = 0; kx < 2; ++kx)
	 {
	    for (j = 0; j < numJ; ++j)
	    {
	       double jIndices[2];
	       GetCornerIndices (jIndices, j, numJ);

	       for (int jx = 0; jx < 2; ++jx)
	       {
		  for (i = 0; i < numI; ++i)
		  {
		     double iIndices[2];
		     GetCornerIndices (iIndices, i, numI);

		     for (int ix = 0; ix < 2; ++ix)
		     {
			double kkk = kIndices[kx];
			double jjj = jIndices[jx];
			double iii = iIndices[ix];

			double depth = GetValue (gridMap, iii, jjj, kkk);
			
			if (doAscii)
			{
            outputFile << (std::isnan( depth ) ? EclipseUndefined : depth) << "  ";
			   if (++numDepth % 4 == 0)
			      outputFile << endl;
			}

			//------------- Binary data
			if (doBinary)
			{
			   fNum = (float) depth;
			   writeValue (fNum, swapMode);

			   checkDataBlocks ();
			}
			++itemCount;
		     }
		  }
	       }
            }
         }
      }
      gridMap->release ();
   }
   if (verbose)
      cout << endl;

   if (debug)
   {
      cerr << "# items to write: " << noItems << ", # items written: " << itemCount << endl;
      cerr << "totalK: " << totalK << endl;
   }

   if (doAscii)
   {
      outputFile << "/" << endl << endl;

      outputFile << "ACTNUM" << endl;
   }

   //------------- Binary data
   if (doBinary)
   {
      writeDelimiter ();
      noItems = numI * numJ * (numberOfHorizons - 1);
      writeKeywordRecord ("ACTNUM", noItems, "INTE");

      writeDelimiter ();
      startDataBlocks (noItems);
   }
   //-------------

   if (verbose)
      cout << "Writing ACTNUM" << endl;

   numDepth = 0;
   int stratIndexEnd = 0;

   for (propertyValueIter = depthPropertyValueList->begin (); propertyValueIter != depthPropertyValueList->end (); ++propertyValueIter)
   {
      const PropertyValue *propertyValue = *propertyValueIter;

      if (propertyValue->getFormation () == 0 || (!basement && propertyValue->getFormation ()->kind () == BASEMENT_FORMATION))
      {
         continue;
      }
      ++stratIndexEnd;

      gridMap = propertyValue->getGridMap ();
      int numK = gridMap->getDepth ();

      if (verbose)
      {
         cout << '-';
         cout.flush ();
      }

      for (k = 0; k < numK; ++k)
      {
         for (j = 0; j < numJ; ++j)
         {
            for (i = 0; i < numI; ++i)
            {

               dNum = 0;
               char charOut = '0';

	       if (gridMap->getValue (i, j, k) != gridMap->getUndefinedValue ())
               {
                  charOut = '1';

                  dNum = 1;
               }

	       if (debug && k == 0)
	       {
		  cerr << charOut;
	       }

               if (doAscii)
               {
                  outputFile << charOut << "  ";

                  if (++numDepth % 4 == 0)
                     outputFile << endl;
               }

               //------------- Binary data
               if (doBinary)
               {
                  writeValue (dNum, swapMode);

                  checkDataBlocks ();
               }
               //-------------
            }
	    if (debug && k == 0)
	       cerr << endl;
         }
	 if (debug && k == 0)
	    cerr << endl << endl;;
      }
   }
   if (verbose)
      cout << endl;
   if (doAscii)
   {
      outputFile << "/" << endl << endl;
   }

   //------------- Binary data
   if (doBinary)
   {
      writeDelimiter ();

      noItems = 0;
      writeKeywordRecord ("ENDGRID", noItems, "INTE");

      writeDelimiter ();

      fclose (fpw);
      sprintf (outputFileNameBin, "%s.INIT", outputFileName.c_str ());
      fpw = fopen (outputFileNameBin, "wb");

      writeINITHeader (swapMode, numI + 1, numJ + 1, numberOfHorizons);
   }
   //-------------

   if (doAscii)
   {
      outputFile << "STRATIGRAPHY" << endl;
   }

   if (verbose)
      cout << "Writing STRATIGRAPHY" << endl;

   numDepth = 0;
   unsigned int stratIndex = 0;

   for (propertyValueIter = depthPropertyValueList->begin (); propertyValueIter != depthPropertyValueList->end (); ++propertyValueIter)
   {
      const PropertyValue *propertyValue = *propertyValueIter;

      if (propertyValue->getFormation () == 0 || (!basement && propertyValue->getFormation ()->kind () == BASEMENT_FORMATION))
      {
         continue;
      }

      ++stratIndex;

      gridMap = propertyValue->getGridMap ();
      int numK = gridMap->getDepth ();

      if (verbose)
      {
         cout << '-';
         cout.flush ();
      }

      for (k = 0; k < numK; ++k)
      {
         for (j = 0; j < numJ; ++j)
         {
            for (i = 0; i < numI; ++i)
            {
               if (doAscii)
               {
                  outputFile << stratIndexEnd + 1 - stratIndex << "  ";
                  if (++numDepth % 4 == 0)
                     outputFile << endl;
               }

               //------------- Binary data
               if (doBinary)
               {
                  dNum = stratIndexEnd + 1 - stratIndex;
                  writeValue (dNum, swapMode);
                  checkDataBlocks ();
               }
               //-------------

            }
         }
      }
   }
   if (verbose)
      cout << endl;
   if (doAscii)
   {
      outputFile << "/" << endl << endl;
   }

   if (!basement)               // to be fixed
   {
      //------------- Binary data
      if (doBinary)
      {
         writeDelimiter ();

         noItems = (numI) * (numJ) * (numberOfHorizons - 1);
         writeKeywordRecord ("LITHOLOG", noItems, "INTE");

         writeDelimiter ();
         startDataBlocks (noItems);
      }
      //-------------

      if (doAscii)
      {
         outputFile << "LITHOLOGY" << endl;
      }

      if (verbose)
         cout << "Writing LITHOLOGY" << endl;

      // Lithology maps are based on the high res input grid, not on the low res output grid
      numDepth = 0;
      for (propertyValueIter = depthPropertyValueList->begin (); propertyValueIter != depthPropertyValueList->end (); ++propertyValueIter)
      {
         const PropertyValue *propertyValue = *propertyValueIter;

         if (propertyValue->getFormation () == 0 || (!basement && propertyValue->getFormation ()->kind () == BASEMENT_FORMATION))
         {
            continue;
         }

         gridMap = propertyValue->getGridMap ();
         int numK = gridMap->getDepth ();

         const Formation *formation = propertyValue->getFormation ();

         assert (formation);

         const LithoType *lithoType1 = formation->getLithoType1 ();
         const LithoType *lithoType2 = formation->getLithoType2 ();
         const LithoType *lithoType3 = formation->getLithoType3 ();
         const LithoType *lithoTypeMax = 0;

         const GridMap *lithoMap1 = 0;
         const GridMap *lithoMap2 = 0;
         const GridMap *lithoMap3 = 0;

         if (lithoType1)
            lithoMap1 = formation->getLithoType1PercentageMap ();
         if (lithoType2)
            lithoMap2 = formation->getLithoType2PercentageMap ();
         if (lithoType3)
            lithoMap3 = formation->getLithoType3PercentageMap ();

         bool allPercentagesConstant = true;

         if ((lithoMap1 && !lithoMap1->isConstant ()) || (lithoMap2 && !lithoMap2->isConstant ()) || (lithoMap3 && !lithoMap3->isConstant ()))
            allPercentagesConstant = false;

         double percent1 = 0;
         double percent2 = 0;
         double percent3 = 0;

         LithologyId::iterator lithologyIndex;
         int lithologyIndexNumber = -1;

         if (allPercentagesConstant)
         {
            if (lithoMap1)
               percent1 = lithoMap1->getConstantValue ();
            if (lithoMap2)
               percent2 = lithoMap2->getConstantValue ();
            if (lithoMap3)
               percent3 = lithoMap3->getConstantValue ();

            // litholog identifier
            if (percent1 >= percent2)
            {
               if (percent1 >= percent3)
               {
                  lithoTypeMax = lithoType1;
               }
               else
               {
                  lithoTypeMax = lithoType3;
               }
            }
            else
            {
               if (percent2 >= percent3)
               {
                  lithoTypeMax = lithoType2;
               }
               else
               {
                  lithoTypeMax = lithoType3;
               }
            }

            const char *lname = lithoTypeMax->getName ().c_str ();

            lithologyIndex = litholog_id.find (lithoTypeMax->getName ());

            if (lithologyIndex == litholog_id.end ())
            {
               double density = 0., heatprod = 0., porosurf = 0.;

               if (percent1 > 0.)
                  density += percent1 * lithoType1->getDensity ();
               if (percent2 > 0.)
                  density += percent2 * lithoType2->getDensity ();
               if (percent3 > 0.)
                  density += percent3 * lithoType3->getDensity ();
               density /= 100.;
               if (percent1 > 0.)
                  heatprod += percent1 * lithoType1->getHeatProduction ();
               if (percent2 > 0.)
                  heatprod += percent2 * lithoType2->getHeatProduction ();
               if (percent3 > 0.)
                  heatprod += percent3 * lithoType3->getHeatProduction ();
               heatprod /= 100.;
               if (percent1 > 0.)
                  porosurf += percent1 * lithoType1->getSurfacePorosity ();
               if (percent2 > 0.)
                  porosurf += percent2 * lithoType2->getSurfacePorosity ();
               if (percent3 > 0.)
                  porosurf += percent3 * lithoType3->getSurfacePorosity ();
               porosurf /= 100.;

               density = lithoTypeMax->getDensity ();
               heatprod = lithoTypeMax->getHeatProduction ();
               porosurf = lithoTypeMax->getSurfacePorosity ();

               lithologyIndexNumber = findLithologId (density, heatprod, porosurf);
            }
            else
            {
               lithologyIndexNumber = lithologyIndex->second;
            }
         }

         const Grid *lithoGrid = 0;

         if (lithoMap1)
            lithoGrid = lithoMap1->getGrid ();
         else if (lithoMap2)
            lithoGrid = lithoMap2->getGrid ();
         else if (lithoMap3)
            lithoGrid = lithoMap3->getGrid ();

         if (verbose)
         {
            cout << '-';
            cout.flush ();
         }

         for (k = 0; k < numK; ++k)
         {
            for (j = 0; j < numJ; ++j)
            {
               for (i = 0; i < numI; ++i)
               {
                  if (!allPercentagesConstant)
                  {
                     // the grid of the lithology gridmaps may be unequal to the output grid
                     // in that case convert from one grid to the other
                     unsigned int lithoI, lithoJ;

                     assert (lithoGrid);
                     if (lithoGrid == grid)
                     {
                        lithoI = i;
                        lithoJ = j;
                     }
                     else
                     {
                        double posI, posJ;
                        bool result;

                        result = grid->getPosition (i, j, posI, posJ);
                        assert (result);
                        result = lithoGrid->getGridPoint (posI, posJ, lithoI, lithoJ);
                        assert (result);
                     }

                     if (lithoMap1)
                        percent1 = lithoMap1->getValue (lithoI, lithoJ);
                     if (lithoMap2)
                        percent2 = lithoMap2->getValue (lithoI, lithoJ);
                     if (lithoMap3)
                        percent3 = lithoMap3->getValue (lithoI, lithoJ);

                     // litholog identifier
                     if (percent1 >= percent2)
                     {
                        if (percent1 >= percent3)
                        {
                           lithoTypeMax = lithoType1;
                        }
                        else
                        {
                           lithoTypeMax = lithoType3;
                        }
                     }
                     else
                     {
                        if (percent2 >= percent3)
                        {
                           lithoTypeMax = lithoType2;
                        }
                        else
                        {
                           lithoTypeMax = lithoType3;
                        }
                     }

                     lithologyIndex = litholog_id.find (lithoTypeMax->getName ());
                     if (lithologyIndex == litholog_id.end ())
                     {
                        double density = 0., heatprod = 0., porosurf = 0.;

                        if (percent1 > 0.)
                           density += percent1 * lithoType1->getDensity ();
                        if (percent2 > 0.)
                           density += percent2 * lithoType2->getDensity ();
                        if (percent3 > 0.)
                           density += percent3 * lithoType3->getDensity ();
                        density /= 100.;
                        if (percent1 > 0.)
                           heatprod += percent1 * lithoType1->getHeatProduction ();
                        if (percent2 > 0.)
                           heatprod += percent2 * lithoType2->getHeatProduction ();
                        if (percent3 > 0.)
                           heatprod += percent3 * lithoType3->getHeatProduction ();
                        heatprod /= 100.;
                        if (percent1 > 0.)
                           porosurf += percent1 * lithoType1->getSurfacePorosity ();
                        if (percent2 > 0.)
                           porosurf += percent2 * lithoType2->getSurfacePorosity ();
                        if (percent3 > 0.)
                           porosurf += percent3 * lithoType3->getSurfacePorosity ();
                        porosurf /= 100.;

                        density = lithoTypeMax->getDensity ();
                        heatprod = lithoTypeMax->getHeatProduction ();
                        porosurf = lithoTypeMax->getSurfacePorosity ();

                        lithologyIndexNumber = findLithologId (density, heatprod, porosurf);
                     }
                     else
                     {
                        lithologyIndexNumber = lithologyIndex->second;
                     }
                  }

                  if (doAscii)
                  {
                     outputFile << lithologyIndexNumber << "  ";
                     if (++numDepth % 4 == 0)
                        outputFile << endl;
                  }

                  //------------- Binary data
                  if (doBinary)
                  {
                     dNum = lithologyIndexNumber;
                     writeValue (dNum, swapMode);

                     checkDataBlocks ();
                  }
                  //------------- 
               }
            }
         }
         gridMap->release ();
      }
      if (verbose)
         cout << endl;
      if (doAscii)
      {
         outputFile << "/" << endl << endl;
      }

   }                            //!basement

   //------------- Binary data
   if (doBinary)
   {
      writeDelimiter ();
   }
   //------------- 


   // process all the properties
   int propIndex;

   for (propIndex = 0; conversions[propIndex].cauldronName != ""; propIndex++)
   {
      PropertyConversion & conversion = conversions[propIndex];
      if (basement && !conversion.hasBasement)
         continue;

      // assume all cauldron properties are node based

      const Property *property = projectHandle->findProperty (conversion.cauldronName);

      if (!property)
      {
         if (debug)
            cerr << "Property " << conversion.cauldronName << " does not exist" << endl;
         continue;
      }

      PropertyValueList *propertyValueList = projectHandle->getPropertyValues (FORMATION, property, snapshot, 0, 0, 0);

      if (propertyValueList->size () == 0)
      {
         if (debug)
            cerr << "Property " << conversion.cauldronName << " has no values" << endl;
         continue;
      }

      if (propertyValueList->size () < stratIndex)
      {
         if (debug)
	    if (basement)
	       cerr << "Property " << conversion.cauldronName << " does not have basement values?" << endl;
	    else
	       cerr << "Property " << conversion.cauldronName << " does not have enough values" << endl;
	 continue;
      }

      if (debug)
         cerr << "Property " << conversion.cauldronName << " has " << propertyValueList->size () << " values" << endl;

      if (verbose)
         cout << "Writing " << conversion.eclipseName << endl;

      if (doAscii)
      {
         outputFile << conversion.eclipseName << endl;
      }

      //------------- Binary data
      if (doBinary)
      {
         sprintf (uWord, "%8s", conversion.eclipseName.c_str ());
         noItems = (numI) * (numJ) * (numberOfHorizons - 1);
         writeKeywordRecord (uWord, noItems, "REAL");

         writeDelimiter ();
         startDataBlocks (noItems);
      }
      //-------------

      for (propertyValueIter = propertyValueList->begin (); propertyValueIter != propertyValueList->end (); ++propertyValueIter)
      {
         const PropertyValue *propertyValue = *propertyValueIter;

         if (propertyValue->getFormation () == 0 || (!basement && propertyValue->getFormation ()->kind () == BASEMENT_FORMATION))
         {
            continue;
         }

         if (debug && propertyValue->getSurface () != 0)
         {
            cerr << "Surface: " << propertyValue->getSurface ()->getName () << endl;
         }

         if (debug && propertyValue->getFormation () != 0)
         {
            cerr << "Formation: " << propertyValue->getFormation ()->getName () << endl;
         }

         gridMap = propertyValue->getGridMap ();
         int numK = gridMap->getDepth ();

         if (verbose)
         {
            cout << '-';
            cout.flush ();
         }
         for (k = 0; k < numK; ++k)
         {
            for (j = 0; j < numJ; ++j)
            {
               for (i = 0; i < numI; ++i)
               {
                  double propValue = gridMap->getValue (i, j, k);

                  if (propValue == gridMap->getUndefinedValue ())
                  {
                     propValue = EclipseUndefined;
                  }
                  else
                  {
                     if (conversion.converter)
                     {
                        propValue = (*conversion.converter) (propValue);
                     }

                  }

                  if (doAscii)
                  {
                     outputFile << propValue << "  ";
                     if (++numDepth % 4 == 0)
                        outputFile << endl;
                  }

                  //------------- Binary data
                  if (doBinary)
                  {
                     fNum = propValue;
                     writeValue (fNum, swapMode);
                     checkDataBlocks ();
                  }
                  //-------------

                  /*
                     if (debug && i == numI / 2 && j == numJ / 2)
                     {
                     cerr << conversion.cauldronName << "(" << k << ", " << i << ", " << j << ") = " << propValue << endl;
                     }
                   */
               }
            }
         }
         gridMap->release ();
      }
      if (verbose)
         cout << endl;

      //------------- Binary data
      if (doBinary)
      {
         writeDelimiter ();
      }
      //-------------

      delete propertyValueList;
      if (doAscii)
      {
         outputFile << "/" << endl << endl;
      }
   }

   if (doAscii)
   {
      outputFile.close ();
   }
   // output binary
   if (doBinary)
   {
      fclose (fpw);
   }

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

   cerr << "Usage (Options may be abbreviated): " << argv0 << " [-verbose] [-snapshot age] [-asciionly] [-binaryonly] [-basement] projectfile" << endl;
   exit (-1);
}
int findLithologId (double density, double heatprod, double porosurf)
{
   int index = 17;

   if (heatprod > 1.3)
   {
      return 10;
   }
   else if (heatprod == 0.0 && density >= 2000.)
   {
      return 16;
   }
   else if (heatprod == 0.0 && density < 2000.)
   {
      return 14;
   }
   else if (density < 2300.)
   {
      return 12;
   }
   else if (heatprod < 0.2)
   {
      return 13;
   }
   else if (heatprod < 0.4)
   {
      return 17;
   }
   else if (heatprod > 0.75 && heatprod < 0.85)
   {
      return 0;
   }
   else if (heatprod * density < 1200)
   {
      return 7;
   }
   else if (density < 2660.)
   {
      return 1;
   }
   else if (density > 2750. || density < 2700.)
   {
      return 7;
   }
   else if (density > 2720.)
   {
      return 16;
   }
   else if (porosurf > 0.6)
   {
      return 8;
   }

   return index;
}

double GetValue (const GridMap * gridMap, double i, double j, double k)
{
   unsigned int iBase = (unsigned int) i;
   unsigned int jBase = (unsigned int) j;
   unsigned int kBase = (unsigned int) k;

   double iFrac = i - iBase;
   double jFrac = j - jBase;
   double kFrac = k - kBase;

   double totalValue = 0;
   double totalWeight = 0;

   for (unsigned int iInc = 0; iInc < 2; ++iInc)
   {
      for (unsigned int jInc = 0; jInc < 2; ++jInc)
      {
	 for (unsigned int kInc = 0; kInc < 2; ++kInc)
	 {
	    double valueAtIndex = gridMap->getValue (iBase + iInc, jBase + jInc, kBase + kInc);
	    if (valueAtIndex != gridMap->getUndefinedValue ())
	    {
	       double weightAtIndex = (iInc == 0 ? 1 - iFrac : iFrac) * (jInc == 0 ? 1 - jFrac : jFrac) * (kInc == 0 ? 1 - kFrac : kFrac);
	       totalValue += weightAtIndex * valueAtIndex;
	       totalWeight += weightAtIndex;
	    }
	 }
      }
   }

   double value;
   
   if (totalWeight == 0)
      value = NAN;
   else
      value = (1 / totalWeight) * totalValue;
      
   return value;
}

// get the petrel corner indices (in a cauldron grid) from a cauldron grid index
void GetCornerIndices (double kIndices[] , int k, int numK)
{
   if (k == 0)
   {
      kIndices[0] = k;
      kIndices[1] = k + 0.5;
   }
   else if (k == numK - 1)
   {
      kIndices[0] = k - 0.5;
      kIndices[1] = k;
   }
   else
   {
      kIndices[0] = k - 0.5;
      kIndices[1] = k + 0.5;
   }
}

static int numItemsLeft;
static int numItemsToWrite;
static int itemCount = 0;
static int dataSize;

void writeDelimiter ()
{
   const bool swapMode = true;
   const int dEndian = 16;
   writeValue (dEndian, swapMode);
}

void writeWord (char * word)
{
   int nlen = strlen (word);
   assert (nlen <= 8);
   fwrite (word, nlen, sizeof (char), fpw);

   for (nlen = nlen; nlen < 8; ++nlen)
      fwrite (" ", 1, sizeof (char), fpw);
}
void writeKeywordRecord (char * keyword, int numberOfElements, char * elementFormat)
{
   const bool swapMode = true;

   int wlen = strlen (keyword);
   fwrite (keyword, wlen, sizeof (char), fpw);
   if (wlen < 8) fwrite ("        ", 8 - wlen, sizeof (char), fpw);

   // number of stream lines
   writeValue (numberOfElements, swapMode);

   wlen = strlen (elementFormat);
   assert (wlen == 4);
   fwrite (elementFormat, wlen, sizeof (char), fpw);
}
void startDataBlocks (int numItems, int size)
{
   const bool swapMode = true;

   dataSize = size;
   numItemsLeft = numItems;
   numItemsToWrite = Min (1000, numItemsLeft);

   // cerr << " starting block of " << numItemsLeft << " of size " << dataSize << endl;

   int dNum = numItemsToWrite * dataSize;
   writeValue (dNum, swapMode);

   // cerr << " starting sub-block of " << numItemsToWrite << " of size " << dataSize << endl;

   itemCount = 0;
}
void checkDataBlocks (void)
{
   const bool swapMode = true;
   int dNum;

   --numItemsLeft;
   ++itemCount;

   if (itemCount == numItemsToWrite)
   {
      // cerr << " finished sub-block of " << numItemsToWrite << " of size " << dataSize << endl;
      dNum = numItemsToWrite * dataSize;
      writeValue (dNum, swapMode);
      numItemsToWrite = Min (1000, numItemsLeft);
      if (numItemsToWrite != 0)
      {
	 dNum = numItemsToWrite * dataSize;
	 writeValue (dNum, swapMode);
	 // cerr << " starting sub-block of " << numItemsToWrite << " of size " << dataSize << endl;
      }
      itemCount = 0;
   }
}

void writeEGRIDHeader( const bool& swapMode, const int& numI, const int& numJ,
      const double& f_posX,  const double& f_posY, const int& numberOfHorizons ) {
   char uWord[9];
   int i, nlen, dNum, noItems = 100;
   writeDelimiter ();

   writeKeywordRecord ("FILEHEAD", noItems, "INTE");

   writeDelimiter ();

   writeValue (noItems * 4, swapMode);
   writeValue (6, swapMode);
   writeValue (2007, swapMode);
   writeValue (0, swapMode);
   writeValue (0, swapMode);
   writeValue (0, swapMode);
   writeValue (0, swapMode);
   writeValue (0, swapMode);

   for (i = 8; i < 101; ++ i) { writeValue (0, swapMode);  }

   writeValue (noItems * 4, swapMode);
   writeDelimiter ();

   noItems = 1;
   writeKeywordRecord ("MAPUNITS", noItems, "CHAR");

   writeDelimiter ();
   writeValue (noItems * 8, swapMode);

   sprintf (uWord, "%s\0", "METRES  ");
   nlen = strlen (uWord);
   fwrite (uWord, nlen, sizeof (char), fpw);

   writeValue (noItems * 8, swapMode);
   writeDelimiter ();

   noItems = 6;
   writeKeywordRecord ("MAPAXES", noItems, "REAL");

   writeDelimiter ();
   writeValue (noItems * 4, swapMode);

   float fxCoorYaxis;
   float fyCoorYaxis;
   float fxCoorOri;
   float fyCoorOri;
   float fxCoorXaxis;
   float fyCoorXaxis;

   fxCoorOri = (float) f_posX;
   fyCoorOri = (float) f_posY;
   fxCoorYaxis = fxCoorOri;
   fyCoorXaxis = fyCoorOri;
   fxCoorXaxis = fxCoorOri + 1000.f;
   fyCoorYaxis = fyCoorOri + 1000.f;

   writeValue (fxCoorYaxis, swapMode);
   writeValue (fyCoorYaxis, swapMode);
   writeValue (fxCoorOri, swapMode);
   writeValue (fyCoorOri, swapMode);
   writeValue (fxCoorXaxis, swapMode);
   writeValue (fyCoorXaxis, swapMode);

   writeValue (noItems * 4, swapMode);
   writeDelimiter ();

   noItems = 2;
   writeKeywordRecord ("GRIDUNIT", noItems, "CHAR");

   writeDelimiter ();
   writeValue (noItems * 8, swapMode);

   sprintf (uWord, "%s\0", "METRES  ");
   nlen = strlen (uWord);
   fwrite (uWord, nlen, sizeof (char), fpw);

   sprintf (uWord, "%s\0", "MAP     ");
   nlen = strlen (uWord);
   fwrite (uWord, nlen, sizeof (char), fpw);

   writeValue (noItems * 8, swapMode);

   writeDelimiter ();

   noItems = 100;
   writeKeywordRecord ("GRIDHEAD", noItems, "INTE");

   writeDelimiter ();

   writeValue ( noItems * 4, swapMode);

   // type of grid - #1
   writeValue (1, swapMode);

   // numI - 1 - #2
   writeValue (numI - 1, swapMode);

   // numJ - 1 - #3
   writeValue (numJ - 1, swapMode);

   // numberOfHorizons - 1 - #4
   writeValue (numberOfHorizons - 1, swapMode);

   // Local Grid Index - #5
   writeValue (0, swapMode);

   // not used Index - #6 - #24
   for (i = 6; i < 25; ++i) { writeValue (0, swapMode); }

   // number of reservoirs - #25
   writeValue (1, swapMode);

   // number of coordinate line segments - #26
   writeValue (1, swapMode);

   // NTHETA - #27
   writeValue (0, swapMode);

   // Lower I-index in host - #28
   writeValue (0, swapMode);

   // Lower J-index in host - #29
   writeValue (numJ - 1, swapMode);

   // Lower K-index in host - #30
   writeValue (0, swapMode);

   // Upper I-index in host - #31
   writeValue (numI - 1, swapMode);

   // Upper J-index in host - #32
   writeValue (0, swapMode);

   // Upper K-index in host - #33
   writeValue (numberOfHorizons - 1, swapMode);

   // not used Index - #34 - #100
   for (i = 34; i < 101; ++ i) { writeValue (0, swapMode); }

   writeValue (noItems * 4, swapMode);
   writeDelimiter ();

   noItems = 6 * numI * numJ;
   writeKeywordRecord ("COORD", noItems, "REAL");


   writeDelimiter ();

   startDataBlocks (noItems);
}

void writeINITHeader( const bool& swapMode, const int& numI, const int& numJ,
      const int& numberOfHorizons )
{
   int i, dNum = 1, noItems = 95;
   writeDelimiter ();

   writeKeywordRecord ("INTEHEAD", noItems, "INTE");

   writeDelimiter ();

   writeValue (noItems * 4, swapMode);

   // integer header array
   for (i = 1; i < noItems + 1; ++i )
   {
      switch( i ){
	 case 3:   dNum = 1; break;
	 case 9:   dNum = numI - 1; break; 
	 case 10:  dNum = numJ - 1; break; 
	 case 11:  dNum = numberOfHorizons - 1; break;
	 case 12:  dNum = (numI - 1) * (numJ - 1) * (numberOfHorizons - 1); break;
	 case -15: dNum = 7;  break;
	 case 65:  
	 case 66:  dNum = 1; break;
	 case 67:  dNum = 2008; break;
	 case 95:  dNum = 100; break;  
	 default: 
		   dNum = 0;
      }
      writeValue (dNum, swapMode);
   }

   writeValue (noItems * 4, swapMode);

   writeDelimiter ();

   noItems = 15;
   writeKeywordRecord ("LOGIHEAD", noItems, "LOGI");

   writeDelimiter ();
   writeValue ( noItems * 4, swapMode);

   writeValue (1, swapMode);
   writeValue (1, swapMode);

   // integer header array
   for (i = 0; i < 13; ++i)
   {
      writeValue (0, swapMode);
   }

   writeValue (noItems * 4, swapMode);
   writeDelimiter ();

   noItems = (numI - 1) * (numJ - 1) * (numberOfHorizons - 1);
   writeKeywordRecord ("STRATIGR", noItems, "INTE");

   writeDelimiter ();
   startDataBlocks (noItems);
}
