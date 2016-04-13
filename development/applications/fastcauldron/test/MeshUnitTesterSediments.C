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
// A simple test for sediments only with no holes.
// There are some zero thickness elements when the time = 10Ma
// these should not be included in the computational domain.
//
TEST ( DofCountingUnitTest, SedimentMesh ) {

   ibs::FilePath fpath( "." );
   fpath << "Acquifer.project3d";

   //std::string ffpath = fpath.fullPath().path();
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

   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);

   // Declaration block required so as to finalise all fastcauldron objects before calling PetscFinalise.
   {
      bool canRunSaltModelling = false;
      std::string errorMessage;
      FastcauldronFactory* factory = new FastcauldronFactory;
      AppCtx *appctx = new AppCtx (argc, argv);
      HydraulicFracturingManager::getInstance ().setAppCtx ( appctx );

      int returnStatus = FastcauldronStartup::startup ( argc, argv, appctx, factory, canRunSaltModelling, errorMessage );

      EXPECT_EQ ( returnStatus, 0 );

      if ( returnStatus == 0 ) {
         // The computational domain consists only of sediments: 0 .. n - 3
         // the last 2 layers on the array are for the cryst and mantle.
         ComputationalDomain domain ( *FastcauldronSimulator::getInstance ().getCauldron ()->layers [ 0 ],
                                      *FastcauldronSimulator::getInstance ().getCauldron ()->layers [ FastcauldronSimulator::getInstance ().getCauldron ()->layers.size () - 3 ],
                                      CompositeElementActivityPredicate ().compose ( ElementActivityPredicatePtr ( new ElementThicknessActivityPredicate )));
         VtkMeshWriter vktWriter;
         MeshUnitTester mut;

         double currentTime;
         std::string testFileName;
         std::string validFileName;

         // First test is for time = 10Ma.
         testFileName = "test_noholes_seds_only_10.vtk";
         validFileName = "valid_noholes_seds_only_10.vtk";
         currentTime = 10.0;
         mut.setTime ( currentTime );
         domain.resetAge ( currentTime );
         vktWriter.save ( domain, testFileName );
         ASSERT_TRUE ( mut.compareFiles ( validFileName, testFileName ));

         // Second test is for time = 0Ma.
         testFileName = "test_noholes_seds_only_0.vtk";
         validFileName = "valid_noholes_seds_only_0.vtk";
         currentTime = 0.0;
         mut.setTime ( currentTime );
         domain.resetAge ( currentTime );
         vktWriter.save ( domain, testFileName );
         ASSERT_TRUE ( mut.compareFiles ( validFileName, testFileName ));
      }

      FastcauldronSimulator::finalise ( false );
      delete factory;
      delete appctx;
   }

   PetscFinalize ();
   free( argv[2] );
   delete[] argv;
}
