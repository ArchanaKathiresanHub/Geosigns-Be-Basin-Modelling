//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// Access to STL library.
#include <string>

// Access to Google test-framework library.
#include <gtest/gtest.h>

// Access to fastcauldron classes.
#include "CompositeElementActivityPredicate.h"
#include "ComputationalDomain.h"
#include "ElementNonZeroPorosityActivityPredicate.h"
#include "ElementThicknessActivityPredicate.h"
#include "FastcauldronFactory.h"
#include "FastcauldronSimulator.h"
#include "FastcauldronStartup.h"
#include "FilePath.h"
#include "HydraulicFracturingManager.h"
#include "layer.h"
#include "propinterface.h"
#include "VtkMeshWriter.h"

// Access to unit testing helper class
#include "MeshUnitTester.h"

//
// A simple test for sediments only with a chess-board like set of holes.
// There are no zero thickness elements.
//
TEST ( DofCountingUnitTest, HoleySedimentMesh01 ) {

   ibs::FilePath fpath( "." );
   fpath << "MeshWithHoles01.project3d";
   std::string ffpath = fpath.path();
   const char* projectName = ffpath.c_str();

   // argc and argv will be used in place of command line 
   // parameters when initialising PETSc and fastcauldron.
   // There are 4 non-null values in the array argv.
   int   argc = 4;
   char** argv = new char*[argc + 1];

   argv [ 0 ] = "fastcauldron";
   argv [ 1 ] = "-project";
   argv [ 2 ] = strdup( projectName );
   argv [ 3 ] = "-decompaction";
   argv [ 4 ] = NULL;

   PetscInitialize (&argc, &argv, (char *) 0, PETSC_IGNORE);

   // Declaration block required so as to finalise all fastcauldron objects before calling PetscFinalise.
   {
      FastcauldronStartup fastcauldronStartup( argc, argv, false, false );
      bool returnStatus = fastcauldronStartup.getPrepareStatus() && fastcauldronStartup.getStartUpStatus();
   
      EXPECT_EQ( returnStatus, true );

      if ( returnStatus )
      {
         // The computational domain consists only of sediments: 0 .. n - 3
         // the last 2 layers on the array are for the cryst and mantle.
         ComputationalDomain domain( *FastcauldronSimulator::getInstance().getCauldron()->layers[0],
            *FastcauldronSimulator::getInstance().getCauldron()->layers[FastcauldronSimulator::getInstance().getCauldron()->layers.size() - 3],
            CompositeElementActivityPredicate().compose( ElementActivityPredicatePtr( new ElementThicknessActivityPredicate ) )
            .compose( ElementActivityPredicatePtr( new ElementNonZeroPorosityActivityPredicate ) ) );
         VtkMeshWriter vktWriter;
         MeshUnitTester mut;

         double currentTime;
         std::string testFileName;
         std::string validFileName;

         testFileName = "test_seds_holes01_0.vtk";
         validFileName = "valid_seds_holes01_0.vtk";
         currentTime = 0.0;
         mut.setTime( currentTime );
         domain.resetAge( currentTime );
         vktWriter.save( domain, testFileName );
         ASSERT_TRUE( mut.compareFiles( validFileName, testFileName ) );
      }

      fastcauldronStartup.finalize( );
   }

   PetscFinalize ();
   free( argv[2] );
   delete [] argv;
}


