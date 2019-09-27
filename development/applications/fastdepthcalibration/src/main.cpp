//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CommonDefinitions.h"
#include "propinterface.h"
#include "FastDepthCalibration.h"
#include "MemoryChecker.h"

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"
#include "ConstantsNumerical.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void abortOnBadAlloc();
void showUsage();
void initializeFastDepthCalibrationLogger(const int rank);

int main(int argc, char** argv)
{
   // If bad_alloc is raised during an allocation of memory then this function will be called.
   std::set_new_handler( abortOnBadAlloc );

   PetscInitialize( &argc, &argv, (char *)0, PETSC_NULL );
   PetscOptionsInsertString("-noofpp");
   PetscOptionsInsertString("-allproperties");

   int rank;
   MPI_Comm_rank( PETSC_COMM_WORLD, &rank );
   Utilities::CheckMemory::MemoryChecker mc(rank);

   try
   {
      initializeFastDepthCalibrationLogger(rank);

      int ierr;
      PetscBool tmp_bool = PETSC_FALSE;

      ierr = PetscOptionsHasName( PETSC_NULL, "-help", &tmp_bool); CHKERRQ( ierr );
      if ( tmp_bool )
      {
        showUsage ();
        PetscFinalize ();
        return -1;
      }

      ierr = PetscOptionsHasName( PETSC_NULL, "-NoCalculatedTWToutput", &tmp_bool); CHKERRQ( ierr );
      bool optionNoCalculatedTWToutput = (tmp_bool==PETSC_TRUE);

      ierr = PetscOptionsHasName( PETSC_NULL, "-preserveErosion", &tmp_bool); CHKERRQ( ierr );
      bool preserveErosion = (tmp_bool==PETSC_TRUE);

      ierr = PetscOptionsHasName( PETSC_NULL, "-NoExtrapolation", &tmp_bool);CHKERRQ( ierr );
      bool noExtrapolation = (tmp_bool==PETSC_TRUE);

      int  optionReferenceSurface;
      ierr = PetscOptionsGetInt( PETSC_NULL, "-referenceSurface", &optionReferenceSurface, PETSC_NULL ); CHKERRQ( ierr );

      int  optionEndSurface;
      ierr = PetscOptionsGetInt( PETSC_NULL, "-endSurface", &optionEndSurface, PETSC_NULL ); CHKERRQ( ierr );

      char projectName[Utilities::Numerical::MaxLineSize];
      projectName[0] = 0;
      ierr = PetscOptionsGetString( PETSC_NULL, "-project", projectName, Utilities::Numerical::MaxLineSize, PETSC_NULL ); CHKERRQ( ierr );

      fastDepthCalibration::FastDepthCalibration fastDepthCalibration( projectName, optionReferenceSurface, optionEndSurface, optionNoCalculatedTWToutput, preserveErosion, noExtrapolation, argc, argv, rank );
      fastDepthCalibration.run();
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CMB API fatal error: " << ex.errorCode() << ", " << ex.what();
      PetscFinalize();
      return 1;
   }
   catch ( const fastDepthCalibration::T2Zexception & ex )
   {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "T2Z fatal error: " << ex.what();
      PetscFinalize();
      return 1;
   }

   PetscFinalize();
   return 0;
}

void abortOnBadAlloc()
{
   LogHandler( LogHandler::FATAL_SEVERITY ) << " cannot allocate resources, aborting";
   MPI_Abort( PETSC_COMM_WORLD, 3 );
}

void showUsage ()
{
   cout << endl;

   cout << "fastdepthcalibration: Perform a depth calibration for the given project file. The tool performs a depth conversion using the given project file as base for the two-way-time maps and lithology definition in formations. It outputs in a folder (T2Zcal_<<the wallclock time>> with the depth converted project file and Inputs.HDF file."<<endl<<endl;

   cout << "Usage: " << endl
        << "\t-project projectname       Name of the project file" << endl
        << "\t-referenceSurface d        The integer d is the reference surface for the calculation. A TWT map needs to be assigned to the formation top. The numbering is the order of formations in the StratIoTbl, starting with 0." << endl
        << "\t-endSurface k              The integer k is the last surface of the depth conversion. The numbering is the order of formations in the StratIoTbl, starting with 0." << endl
        << "\t-<<run mode>>              A cauldron run mode must be specified for the calculations. Like the fastcauldron run mode, e.g., -temperature or -itcoupled" << endl
        << "\t[-preserveErosion]         If this flag is set the tool will preserve erosion in the base project file. The tool will only identify erosions which also have zero TWT difference at the erosion location! (wrong/unwanted micro erosions might still disapear)" << endl
        << "\t[-NoCalculatedTWToutput]   If this flag is set the caluclated time maps which are generated for missing TWT maps in the inputs are NOT written to the resulting project file. The default setting is, that these TWT maps will be written in the output file (marked as calculated TWT maps)" << endl
        << "\t[-NoExtrapolation]         If this flag is set there will be no extrapolation of data at the edge of the domain (only in subsampled mode, the result is a blocky looking AOI)" << endl
        << "\t[-help]                    Shows this help message and exit." << endl << endl;
}

void initializeFastDepthCalibrationLogger(const int rank)
{
  PetscBool log = PETSC_FALSE;
  PetscOptionsHasName( PETSC_NULL, "-verbosity", &log );
  if ( log )
  {
    char verbosity[11];
    PetscOptionsGetString( PETSC_NULL, "-verbosity", verbosity, 11, 0 );

    if (      !strcmp( verbosity, "quiet"      ) ) { LogHandler( "fastdepthcalibration", LogHandler::QUIET_LEVEL,      rank ); }
    else if ( !strcmp( verbosity, "minimal"    ) ) { LogHandler( "fastdepthcalibration", LogHandler::MINIMAL_LEVEL,    rank ); }
    else if ( !strcmp( verbosity, "normal"     ) ) { LogHandler( "fastdepthcalibration", LogHandler::NORMAL_LEVEL,     rank ); }
    else if ( !strcmp( verbosity, "detailed"   ) ) { LogHandler( "fastdepthcalibration", LogHandler::DETAILED_LEVEL,   rank ); }
    else if ( !strcmp( verbosity, "diagnostic" ) ) { LogHandler( "fastdepthcalibration", LogHandler::DIAGNOSTIC_LEVEL, rank ); }
    else { throw fastDepthCalibration::T2Zexception() << "Unknown <" << verbosity << "> option for -verbosity command line parameter."; }
  }
  else { LogHandler( "fastdepthcalibration", LogHandler::DETAILED_LEVEL, rank ); }
}
