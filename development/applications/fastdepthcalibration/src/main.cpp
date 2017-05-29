//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "FastcauldronSimulator.h"
#include "FastDepthCalibration.h"
#include "MemoryChecker.h"

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"
#include "ConstantsNumerical.h"
using Utilities::Numerical::MaxLineSize;

static void abortOnBadAlloc()
{
   LogHandler( LogHandler::FATAL_SEVERITY ) << " cannot allocate resources, aborting";
   MPI_Abort( PETSC_COMM_WORLD, 3 );
}

void addNoofppToCommandLine ();


int main(int argc, char** argv)
{

   // If bad_alloc is raised during an allocation of memory then this function will be called.
   std::set_new_handler( abortOnBadAlloc );

   // Initialise Petsc and get rank & size of MPI
   PetscInitialize( &argc, &argv, (char *)0, PETSC_NULL );

   // If -noofpp is not a command line parameter then add it.
   addNoofppToCommandLine ();

   int rank;
   MPI_Comm_rank( PETSC_COMM_WORLD, &rank );
   Utilities::CheckMemory::MemoryChecker mc(rank);

   try
   {
      // Intitialise fastcauldron logger (it is a singleton, must be initialize only once!)
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
         else { throw T2Zexception() << "Unknown <" << verbosity << "> option for -verbosity command line parameter."; }
      }
      else { LogHandler( "fastdepthcalibration", LogHandler::DETAILED_LEVEL, rank ); }

      // read command line options
      int  optionReferenceSurface;
      int  optionEndSurface;
      char projectName[MaxLineSize];
      int  ierr;

      projectName [ 0 ] = 0;

      ierr = PetscOptionsGetInt(    PETSC_NULL, "-referenceSurface", &optionReferenceSurface, PETSC_NULL );     CHKERRQ( ierr );
      ierr = PetscOptionsGetInt(    PETSC_NULL, "-endSurface",       &optionEndSurface,       PETSC_NULL );     CHKERRQ( ierr );
      ierr = PetscOptionsGetString( PETSC_NULL, "-project",          projectName, MaxLineSize, 0         );     CHKERRQ( ierr );

      // run fastdepthCalibration
      FastDepthCalibration fastDepthCalibration( projectName, optionReferenceSurface, optionEndSurface, argc, argv, rank );
	  fastDepthCalibration.calculateInitialMaps();
	  fastDepthCalibration.calibrateDepths();
	  fastDepthCalibration.writeFinalProject();

   }
   catch ( const ErrorHandler::Exception & ex )
   {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CMB API fatal error: " << ex.errorCode() << ", " << ex.what();
      PetscFinalize(); // Close Petsc
      return 1;
   }
   catch ( const T2Zexception & ex )
   {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "T2Z fatal error: " << ex.what();
      PetscFinalize(); // Close Petsc
      return 1;
   }

   // Close Petsc
   PetscFinalize();
   return 0;
}

void addNoofppToCommandLine () {

   PetscBool hasNoOfpp = PETSC_FALSE;
   char noOfppCommandLine [] = "-noofpp";

   PetscOptionsHasName( PETSC_NULL, noOfppCommandLine, &hasNoOfpp );

   if ( hasNoOfpp == PETSC_FALSE ) {
      PetscOptionsInsertString ( noOfppCommandLine );
   }

}
