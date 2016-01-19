//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <iostream>
#include <fstream>

// petsc library
#include <petsc.h>

// CrustalThickness library
#include "CrustalThicknessCalculator.h"
#include "CrustalThicknessCalculatorFactory.h"

// utilitites
#include "LogHandler.h"

// Flexlm licenses
#ifdef FLEXLM
#undef FLEXLM
#endif

#ifdef DISABLE_FLEXLM
#undef FLEXLM
#else
#define FLEXLM 1
#endif

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif

//------------------------------------------------------------//
void showUsage () {
   cout << endl;

   cout << "Usage: " << endl
        << "\t-project projectname       Name of the project file" << endl
        << "\t[-debug]                   Output all map properties. Use in combination with -hdf or/and -xyz or/and -sur to output into individual files" << endl
        << "\t[-nosmooth]                Don't smooth the result maps" << endl
        << "\t[-smooth <radius>]         Smooth the result maps using the defined <radius>. Default value: 5" << endl
        << "\t[-save filename]           Name of output project file" << endl
        << "\t[-xyz]                     Output selected maps also in XYZ format" << endl
        << "\t[-sur]                     Use only in combination with -debug. Output selected maps in SUR format (to visualize surface chart in Excel)" << endl
        << "\t[-hdf]                     Use only in combination with -debug. Output all maps in separate HDF files." << endl
        << "\t[-help]                    Shows this help message and exit." << endl << endl;
}

//------------------------------------------------------------//
void finaliseCrustalThicknessCalculator ( char* feature, const char* errorMessage = "" , CrustalThicknessCalculatorFactory* factory = 0 ) {
   
   if ( strlen ( errorMessage ) > 0 ) {
      PetscPrintf ( PETSC_COMM_WORLD, "\nMeSsAgE ERROR %s \n\n", errorMessage );
   }

 
   CrustalThicknessCalculator::finalise (false);

   if ( factory != 0 ) {
      delete factory;
   }

#ifdef FLEXLM
   if( ddd::GetRank() == 0 ) {
      EPTFlexLmCheckIn( feature );
      EPTFlexLmTerminate();
   }
#endif

   PetscFinalize ();
}

//------------------------------------------------------------//
int main (int argc, char ** argv)
{
   bool status;
   int rank = 99999;
   typedef formattingexception::GeneralException CtcException;

   CrustalThicknessCalculatorFactory* factory = new CrustalThicknessCalculatorFactory;
   //DataAccess::Interface::ProjectHandle::UseFactory (factory);

   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);

   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

#ifdef FLEXLM
   int rc = EPTFLEXLM_OK;
   
   char feature[EPTFLEXLM_MAX_FEATURE_LEN];
   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];
   

   // FlexLM license handling only for node with rank = 0
   if( rank == 0 ) {
      sprintf(feature, "ibs_cauldron_calc");
#ifdef IBSFLEXLMVERSION
      sprintf(version, IBSFLEXLMVERSION);
#else
      sprintf(version, "9999.99");
#endif

      rc = EPTFlexLmInit(errmessage);
      if ( rc != EPTFLEXLM_OK ) {
         fprintf(stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
      }
      // FlexLM license handling: Checkout
      rc = EPTFlexLmCheckOut(feature, version, errmessage);
      if (rc == EPTFLEXLM_WARN) {
         fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license warning: fastcauldron will still start anyway.\n@@@@@@@@@@@@@@@\n");
      } else if (rc != EPTFLEXLM_OK) {
         fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license error: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
      }
   }
   
   MPI_Bcast ( &rc, 1, MPI_INT, 0, PETSC_COMM_WORLD);
   
   
   if( rc != EPTFLEXLM_OK && rc != EPTFLEXLM_WARN) {
      PetscFinalize ();
#ifdef FLEXLM
      //FlexLM license check in only for node with rank = 0
      if( rank == 0 ) {
         // FlexLm license check in, close down and enable logging
         EPTFlexLmCheckIn( feature );
         EPTFlexLmTerminate();
      }
#endif
      
      return -1;
   }
#else
   char feature[] = "ibs_cauldron_calc";
#endif 

#ifndef _MSC_VER
   PetscBool myddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-myddd", &myddd);
   if (myddd)
   {
      char cmd[150];

      sprintf (cmd, "myddd  %s %d &", argv[0],  getpid ());
      
      system (cmd);
      sleep (20);
   }
#endif

   /// @todo make command line argument for verbosity
   LogHandler ctcLog( "CTC", LogHandler::DIAGNOSTIC_LEVEL, rank );

   PetscBool isDefined = PETSC_FALSE;
   
   PetscOptionsHasName (PETSC_NULL, "-help", &isDefined);
   if (isDefined) {
      showUsage ();
      PetscFinalize ();
      return -1;
   }

   const int lineSize = 128;
   char inputFileName[lineSize];
   inputFileName[0] = '\0';

   PetscOptionsGetString (PETSC_NULL, "-project", inputFileName, lineSize, &isDefined);

   if (!isDefined)  {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "ERROR Error when reading the project file";
      fprintf(stderr, "MeSsAgE ERROR Error when reading the project file\n");
      showUsage ();
      PetscFinalize ();
      return -1;
   }

   PetscLogDouble sim_Start_Time;
   PetscTime( &sim_Start_Time );

   if (!CrustalThicknessCalculator::CreateFrom( inputFileName, factory )) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Can not open the project file";
      fprintf(stderr, "MeSsAgE ERROR Can not open the project file\n");
      showUsage ();
      PetscFinalize ();
      return -1;
   };

   if( !CrustalThicknessCalculator::getInstance().parseCommandLine()) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not parse command line";
      finaliseCrustalThicknessCalculator(feature, "", factory);
      return -1;
   };
   
   try {
      CrustalThicknessCalculator::getInstance().deleteCTCPropertyValues();

      CrustalThicknessCalculator::getInstance().run();
         
      }
   /// @todo delete catch string once migrated to CtcException
   catch ( std::string& s ) {
      finaliseCrustalThicknessCalculator(feature, s.c_str(), factory);
      return 0;
   }
   catch (CtcException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
      finaliseCrustalThicknessCalculator( feature, ex.what(), factory );
      return 0;
   }
   catch (...) {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CTC fatal error";
      finaliseCrustalThicknessCalculator(feature, "", factory);
      return 0;
   }

   PetscLogDouble sim_End_Time;
   PetscTime( &sim_End_Time );
   displayTime( sim_End_Time - sim_Start_Time, "End of simulation" );

#ifdef FLEXLM
   //FlexLM license check in only for node with rank = 0
   if( rank == 0 ) {
      // FlexLm license check in, close down and enable logging
      EPTFlexLmCheckIn( feature );
      EPTFlexLmTerminate();
   }
#endif

   CrustalThicknessCalculator::finalise(true);

   PetscTime( &sim_End_Time );   
   displayTime( sim_End_Time - sim_Start_Time, "Total time" );
     
   delete factory;

   PetscFinalize ();

   return 0;
}

