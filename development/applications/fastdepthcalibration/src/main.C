//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <assert.h>
#include <stdlib.h>
#include "FastcauldronSimulator.h"
#include "FastDepthCalibration.h"
#include "MemoryChecker.h"

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"

static void abortOnBadAlloc()
{
   LogHandler( LogHandler::FATAL_SEVERITY ) << " cannot allocate resources, aborting";
   MPI_Abort( PETSC_COMM_WORLD, 3 );
}

int main(int argc, char** argv)
{
   // If bad_alloc is raised during an allocation of memory then this function will be called.
   std::set_new_handler( abortOnBadAlloc );
   MemoryChecker mc;

   // Initialise Petsc and get rank & size of MPI
   PetscInitialize( &argc, &argv, (char *)0, PETSC_NULL );

   int rank;
   MPI_Comm_rank( PETSC_COMM_WORLD, &rank );

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
      char projectName[MAXLINESIZE];
      int  ierr;
    
      ierr = PetscOptionsGetInt(    PETSC_NULL, "-referenceSurface", &optionReferenceSurface, PETSC_NULL );     CHKERRQ( ierr );
      ierr = PetscOptionsGetInt(    PETSC_NULL, "-endSurface",       &optionEndSurface,       PETSC_NULL );     CHKERRQ( ierr );
      ierr = PetscOptionsGetString( PETSC_NULL, "-project",          projectName,             MAXLINESIZE, 0 ); CHKERRQ( ierr );

      
      FastDepthCalibration fastDepthCalibration( projectName, optionReferenceSurface, optionEndSurface, argc, argv, rank );
      
      if ( ErrorHandler::NoError != fastDepthCalibration.calculateInitialMaps( ) )
      {
         throw T2Zexception( ) << "Error in calculateInitialMaps: " << ", " << fastDepthCalibration.errorMessage( );
      }

      if ( ErrorHandler::NoError != fastDepthCalibration.calibrateDepths( ) )
      {
         throw T2Zexception( ) << "Error in calibrateDepths: " << ", " << fastDepthCalibration.errorMessage( );
      }

      if ( ErrorHandler::NoError != fastDepthCalibration.writeFinalProject( ) )
      {
         throw T2Zexception( ) << "Error in writeFinalProject: " << ", " << fastDepthCalibration.errorMessage( );
      }
      
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
