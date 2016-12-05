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
#include "StatisticsHandler.h"

#include "MemoryChecker.h"
#include "FastcauldronStartup.h"

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"


static void abortOnBadAlloc () {
   cerr << " cannot allocate ersources, aborting"  << endl;
   MPI_Abort ( PETSC_COMM_WORLD, 3 );
}

int main(int argc, char** argv)
{
   bool returnStatus = true;
   bool canRunSaltModelling = true;

   // If bad_alloc is raised during an allocation of memory then this function will be called.
   std::set_new_handler ( abortOnBadAlloc );

   // Initialise Petsc and get rank & size of MPI
   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);
   int rank;
   MPI_Comm_rank( PETSC_COMM_WORLD, &rank );
   Utilities::CheckMemory::MemoryChecker mc(rank);
   
   // Intitialise fastcauldron loger
   try
   {
      PetscBool log = PETSC_FALSE;
      PetscOptionsHasName( PETSC_NULL, "-verbosity", &log );
      if ( log )
      {
         char verbosity[11];
         PetscOptionsGetString( PETSC_NULL, "-verbosity", verbosity, 11, 0 );
         if      ( !strcmp( verbosity, "quiet"      ))  { LogHandler( "fastcauldron", LogHandler::QUIET_LEVEL     , rank ); }
         else if ( !strcmp( verbosity, "minimal"    ) ) { LogHandler( "fastcauldron", LogHandler::MINIMAL_LEVEL   , rank ); }
         else if ( !strcmp( verbosity, "normal"     ) ) { LogHandler( "fastcauldron", LogHandler::NORMAL_LEVEL    , rank ); }
         else if ( !strcmp( verbosity, "detailed"   ) ) { LogHandler( "fastcauldron", LogHandler::DETAILED_LEVEL  , rank ); }
         else if ( !strcmp( verbosity, "diagnostic" ) ) { LogHandler( "fastcauldron", LogHandler::DIAGNOSTIC_LEVEL, rank ); }
         else throw formattingexception::GeneralException() << "Unknown <" << verbosity << "> option for -verbosity command line parameter.";
      }
      else
      {
         LogHandler( "fastcauldron", LogHandler::DETAILED_LEVEL, rank );
      }
   }
   catch ( formattingexception::GeneralException & ex )
   {
      std::cerr << ex.what();
      return 1;
   }
   catch (...)
   {
      std::cerr << "Fatal error when initialising log file(s).";
      return 1;
   }

   
   try
   {
      FastcauldronStartup fastcauldronStartup( argc, argv );
      fastcauldronStartup.run();
      fastcauldronStartup.finalize( );
      returnStatus = fastcauldronStartup.getPrepareStatus() && fastcauldronStartup.getStartUpStatus() && fastcauldronStartup.getRunStatus();
   }
   catch ( std::exception& e )
   {
      LogHandler( LogHandler::ERROR_SEVERITY ) << e.what( );
      return 1;
   }
   catch ( ... )
   {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Unknown exception occured.\n";
      return 1;
   }

   PetscFinalize();
   
   return returnStatus ? 0 : 1;
}

