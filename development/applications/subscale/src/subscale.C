#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#ifdef sgi
#include <string.h>
#include <iostream.h>
#else
#include <string>
#include <iostream>
#endif

#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include <vector>
#include <map>

using namespace std;
using namespace database;

DataSchema * createInputCauldronSchema ();

int main (int argc, char ** argv)
{
   string fileIn;
   string fileOut;

   int scaleX = 1;
   int scaleY = 1;
   bool perfTester = false;

   int i;
   for (i = 1; i < argc; i++)
   {
      if ( strncmp(argv[i], "-pt", Max( 2, strlen(argv[i]) )) == 0 )
      {
         perfTester = true;
      }
      else if (strncmp (argv[i], "-input", Max (2, strlen (argv[i]))) == 0)
      {
	 fileIn = argv[++i];
      }
      else if (strncmp (argv[i], "-output", Max (2, strlen (argv[i]))) == 0)
      {
	 fileOut = argv[++i];
      }
      else if (strncmp (argv[i], "-x", Max (2, strlen (argv[i]))) == 0)
      {
	 scaleX = atoi (argv[++i]);
      }
      else if (strncmp (argv[i], "-y", Max (2, strlen (argv[i]))) == 0)
      {
	 scaleY = atoi (argv[++i]);
      }
      else
      {
         cerr << "Invalid parameter (" << argv[i] << "). Usage: " << argv[0] << " -pt -input fileIn -output fileOut [-x xxx] [-y yyy]" << endl;
	 return -1;
      }
   }

   if (fileIn == "" || fileOut == "")
   {
      cerr << "No input or output file. Usage: " << argv[0] << " -pt -input fileIn -output fileOut [-x xxx] [-y yyy]" << endl;
      return -1;
   }

   DataSchema * projSchema = createInputCauldronSchema ();
   Database * projBase = Database::CreateFromFile (fileIn, * projSchema);
   delete projSchema;
   if (!projBase)
   {
      cerr << "Failed to read project file " << fileIn << endl;
      return -1;
   }

   Table * projectIoTbl = projBase->getTable ("ProjectIoTbl");

   if (projectIoTbl)
   {
      Record * projectIoRecord = projectIoTbl->getRecord (0);
      if (projectIoRecord)
      {
        if ( perfTester == false )
          {
            setScaleX (projectIoRecord, scaleX);
            setScaleY (projectIoRecord, scaleY);
          }
        else
          {
            // force non geometric loop 
            Table * runOptionsIoTbl = projBase->getTable ("RunOptionsIoTbl");
            Record * runOptionsIoRecord = runOptionsIoTbl->getRecord (0);
            const int val = 1;
            setNonGeometricLoop (runOptionsIoRecord, val );

            // set x values
            // must be able to permit models generated to run on 64 processors
            int sizeX = getNumberX ( projectIoRecord );
            if ( sizeX < 16 * scaleX )
              {
                cerr << "Could not subscale file " << fileIn << " the X size was not big enough for subsampling requested." << endl;
                return -1;
              }
            setScaleX ( projectIoRecord, scaleX );
            setXCoord ( projectIoRecord, 0 );
            setOffsetX ( projectIoRecord, 0 );
            setWindowXMin ( projectIoRecord, 0 );
            setWindowXMax ( projectIoRecord, sizeX - 1 );

            // set y values
            int sizeY = getNumberY ( projectIoRecord );
            if ( sizeY < 16 * scaleY )
              {
                cerr << "Could not subscale file " << fileIn << " the Y size was not big enough for subsampling requested." << endl;
                return -1;
              }
            setScaleY ( projectIoRecord, scaleY );
            setYCoord ( projectIoRecord, 0 );
            setOffsetY ( projectIoRecord, 0 );
            setWindowYMin ( projectIoRecord, 0 );
            setWindowYMax ( projectIoRecord, sizeY - 1 );
          }
      }
   }

   projBase->saveToFile (fileOut);

   delete projBase;
   return 0;
}


DataSchema * createInputCauldronSchema ()
{
   DataSchema * dataSchema = new DataSchema;

   createProjectIoTblDefinition (dataSchema);
   createProjectNotesIoTblDefinition (dataSchema);
   createRelatedProjectsIoTblDefinition (dataSchema);
   createDefaultFileIoTblDefinition (dataSchema);
   createIoOptionsIoTblDefinition (dataSchema);
   createRunOptionsIoTblDefinition (dataSchema);
   createSnapshotIoTblDefinition (dataSchema);
   createSensAnalysisIoTblDefinition (dataSchema);
   createRespSurfModelParamsIoTblDefinition (dataSchema);
   createFilterDepthIoTblDefinition (dataSchema);
   createFilterTimeIoTblDefinition (dataSchema);
   createFilterTimeDepthIoTblDefinition (dataSchema);
   createWellLocIoTblDefinition (dataSchema);
   createTouchstoneWellIoTblDefinition (dataSchema);
   createGridMapIoTblDefinition (dataSchema);
   createStratIoTblDefinition (dataSchema);
   createMobLayThicknIoTblDefinition (dataSchema);
   createCrustIoTblDefinition (dataSchema);
   createSourceRockLithoIoTblDefinition (dataSchema);
   createGeologicalBoundaryIoTblDefinition (dataSchema);
   createPropertyBoundaryIoTblDefinition (dataSchema);
   createBoundaryValuesIoTblDefinition (dataSchema);
   createSurfaceTempIoTblDefinition (dataSchema);
   createSurfaceDepthIoTblDefinition (dataSchema);
   createMntlHeatFlowIoTblDefinition (dataSchema);
   createBasementIoTblDefinition (dataSchema);
   createFormPressIoTblDefinition (dataSchema);
   createFormTempIoTblDefinition (dataSchema);
   createRawBhtIoTblDefinition (dataSchema);
   createRawBhtBitsizeIoTblDefinition (dataSchema);
   createMarsdenCoefIoTblDefinition (dataSchema);
   createVrmIoTblDefinition (dataSchema);
   createSonicIoTblDefinition (dataSchema);
   createBulkDensityIoTblDefinition (dataSchema);
   createBiomarkerKinIoTblDefinition (dataSchema);
   createBiomarkermIoTblDefinition (dataSchema);
   createSmectiteIlliteKinIoTblDefinition (dataSchema);
   createSmectiteIlliteIoTblDefinition (dataSchema);
   createFtGrainIoTblDefinition (dataSchema);
   createFtLengthIoTblDefinition (dataSchema);
   createFtSampleIoTblDefinition (dataSchema);
   createReservoirIoTblDefinition (dataSchema);
   createBioDegradIoTblDefinition (dataSchema);
   createDiffusionIoTblDefinition (dataSchema);
   createPalinspasticIoTblDefinition (dataSchema);
   createFaultcutIoTblDefinition (dataSchema);
   createPressureFuncIoTblDefinition (dataSchema);
   createSourceRockPropIoTblDefinition (dataSchema);
   createReservoirPropIoTblDefinition (dataSchema);
   createGridIoTblDefinition (dataSchema);
   createReservoirLeakIoTblDefinition (dataSchema);
   createLithotypeIoTblDefinition (dataSchema);
   createLitMixIoTblDefinition (dataSchema);
   createLitThCondIoTblDefinition (dataSchema);
   createLitHeatCapIoTblDefinition (dataSchema);
   createFluidtypeIoTblDefinition (dataSchema);
   createFltThCondIoTblDefinition (dataSchema);
   createFltHeatCapIoTblDefinition (dataSchema);
   createFltDensityIoTblDefinition (dataSchema);
   createFltViscoIoTblDefinition (dataSchema);
   createDisplayContourIoTblDefinition (dataSchema);
   createPlotBoundsTblDefinition (dataSchema);
   createRunStatusIoTblDefinition (dataSchema);
   createExportDepthIoTblDefinition (dataSchema);
   createExportTimeIoTblDefinition (dataSchema);
   createHistogramMCResultIoTblDefinition (dataSchema);
   createMarkerMCResultIoTblDefinition (dataSchema);
   createTemperatureIsoIoTblDefinition (dataSchema);
   createVrIsoIoTblDefinition (dataSchema);
   createTouchstoneMapIoTblDefinition (dataSchema);
   createGrpCombIoTblDefinition (dataSchema);
   createFTmIoTblDefinition (dataSchema);

   return dataSchema;
}
