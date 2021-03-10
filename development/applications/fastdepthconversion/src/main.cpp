//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CommonDefinitions.h"
#include "FastDepthConversion.h"
#include "MemoryChecker.h"

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"
#include "ConstantsNumerical.h"

#include <petscsys.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void abortOnBadAlloc();
void showUsage();
void initializeFastDepthConversionLogger(const int rank);

int main(int argc, char** argv)
{
  // If bad_alloc is raised during an allocation of memory then this function will be called.
  std::set_new_handler( abortOnBadAlloc );

  PetscInitialize( &argc, &argv, (char *)0, PETSC_IGNORE );
  PetscOptionsInsertString(PETSC_IGNORE, "-noofpp");
  PetscOptionsInsertString(PETSC_IGNORE, "-allproperties");

  try
  {
    int rank;
    MPI_Comm_rank( PETSC_COMM_WORLD, &rank );
    Utilities::CheckMemory::MemoryChecker mc(rank);

    initializeFastDepthConversionLogger(rank);

    int ierr;
    PetscBool tmp_bool = PETSC_FALSE;

    ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-help", &tmp_bool); CHKERRQ( ierr );

    if ( tmp_bool==PETSC_TRUE )
    {
      if ( rank == 0 )
      {
        showUsage();
      }
      PetscFinalize();
      return -1;
    }

    ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-NoCalculatedTWToutput", &tmp_bool); CHKERRQ( ierr );
    bool optionNoCalculatedTWToutput = (tmp_bool==PETSC_TRUE);

    ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-preserveErosion", &tmp_bool); CHKERRQ( ierr );
    bool preserveErosion = (tmp_bool==PETSC_TRUE);

    ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-NoExtrapolation", &tmp_bool);CHKERRQ( ierr );
    bool noExtrapolation = (tmp_bool==PETSC_TRUE);

    int  optionReferenceSurface;
    ierr = PetscOptionsGetInt(PETSC_IGNORE, PETSC_IGNORE, "-referenceSurface", &optionReferenceSurface, PETSC_IGNORE ); CHKERRQ( ierr );

    int  optionEndSurface;
    ierr = PetscOptionsGetInt(PETSC_IGNORE, PETSC_IGNORE, "-endSurface", &optionEndSurface, PETSC_IGNORE ); CHKERRQ( ierr );

    char projectName[Utilities::Numerical::MaxLineSize];
    projectName[0] = 0;
    ierr = PetscOptionsGetString(PETSC_IGNORE, PETSC_IGNORE, "-project", projectName, Utilities::Numerical::MaxLineSize, PETSC_IGNORE ); CHKERRQ( ierr );

    fastDepthConversion::FastDepthConversion fastDepthConversion( projectName, optionReferenceSurface, optionEndSurface, optionNoCalculatedTWToutput, preserveErosion, noExtrapolation, argc, argv, rank );
    fastDepthConversion.run();
  }
  catch ( const ErrorHandler::Exception & ex )
  {
    LogHandler( LogHandler::FATAL_SEVERITY ) << "CMB API fatal error: " << ex.errorCode() << ", " << ex.what();
    PetscFinalize();
    return 1;
  }
  catch ( const fastDepthConversion::T2Zexception & ex )
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
  LogHandler( LogHandler::INFO_SEVERITY ) << "fastdepthconversion: Perform a depth conversion for the given project file. The tool performs a depth conversion using the given project file as base for the two-way-time maps and lithology definition in formations. It outputs in a folder (T2Zcal_<<the wallclock time>> with the depth converted project file and Inputs.HDF file.\n";

  LogHandler( LogHandler::INFO_SEVERITY ) << "Usage: \n"
                                          << "\t-project projectname       Name of the project file\n"
                                          << "\t-referenceSurface d        The integer d is the reference surface for the calculation. A TWT map needs to be assigned to the formation top. The numbering is the order of formations in the StratIoTbl, starting with 0.\n"
                                          << "\t-endSurface k              The integer k is the last surface of the depth conversion. The numbering is the order of formations in the StratIoTbl, starting with 0.\n"
                                          << "\t-<<run mode>>              A cauldron run mode must be specified for the calculations. Like the fastcauldron run mode, e.g., -temperature or -itcoupled\n"
                                          << "\t[-preserveErosion]         If this flag is set the tool will preserve erosion in the base project file. The tool will only identify erosions which also have zero TWT difference at the erosion location! (wrong/unwanted micro erosions might still disapear)\n"
                                          << "\t[-NoCalculatedTWToutput]   If this flag is set the caluclated time maps which are generated for missing TWT maps in the inputs are NOT written to the resulting project file. The default setting is, that these TWT maps will be written in the output file (marked as calculated TWT maps)\n"
                                          << "\t[-NoExtrapolation]         If this flag is set there will be no extrapolation of data at the edge of the domain (only in subsampled mode, the result is a blocky looking AOI)\n"
                                          << "\t[-help]                    Shows this help message and exit.\n";
}

void initializeFastDepthConversionLogger(const int rank)
{
  PetscBool log = PETSC_FALSE;
  PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-verbosity", &log );
  if ( log )
  {
    char verbosity[11];
    PetscOptionsGetString(PETSC_IGNORE, PETSC_IGNORE, "-verbosity", verbosity, 11, 0 );

    if (      !strcmp( verbosity, "quiet"      ) ) { LogHandler( "fastdepthconversion", LogHandler::QUIET_LEVEL,      rank ); }
    else if ( !strcmp( verbosity, "minimal"    ) ) { LogHandler( "fastdepthconversion", LogHandler::MINIMAL_LEVEL,    rank ); }
    else if ( !strcmp( verbosity, "normal"     ) ) { LogHandler( "fastdepthconversion", LogHandler::NORMAL_LEVEL,     rank ); }
    else if ( !strcmp( verbosity, "detailed"   ) ) { LogHandler( "fastdepthconversion", LogHandler::DETAILED_LEVEL,   rank ); }
    else if ( !strcmp( verbosity, "diagnostic" ) ) { LogHandler( "fastdepthconversion", LogHandler::DIAGNOSTIC_LEVEL, rank ); }
    else { throw fastDepthConversion::T2Zexception() << "Unknown <" << verbosity << "> option for -verbosity command line parameter."; }
  }
  else { LogHandler( "fastdepthconversion", LogHandler::DETAILED_LEVEL, rank ); }
}
