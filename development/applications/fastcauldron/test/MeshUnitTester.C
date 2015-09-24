//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "MeshUnitTester.h"

// Access to STL library.
#include <iostream>

// Access to fastcauldron application code.
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "StatisticsHandler.h"
#include "layer.h"
#include "layer_iterators.h"
#include "propinterface.h"

bool MeshUnitTester::setTime ( const double currentTime ) const {

   using namespace Basin_Modelling;

   AppCtx* cauldron = const_cast<AppCtx*>(FastcauldronSimulator::getInstance ().getCauldron ());
   bool status = true;

   status &= cauldron->findActiveElements( currentTime );

   Layer_Iterator layers;
   LayerProps* currentLayer;
   layers.Initialise_Iterator ( cauldron->layers, Descending, Basement_And_Sediments, 
                                Active_Layers_Only );

   while ( ! layers.Iteration_Is_Done ()) {
      currentLayer = layers.Current_Layer();
      status &= currentLayer->allocateNewVecs ( cauldron, currentTime );
      layers++;
   }

   FastcauldronSimulator::getInstance ().switchLithologies ( currentTime );

   status &= cauldron->calcNodeVes( currentTime );
   PETSC_ASSERT( status );

   status &= cauldron->calcNodeMaxVes( currentTime );
   PETSC_ASSERT( status );

   status &= cauldron->calcPorosities(  currentTime  );
   PETSC_ASSERT( status );

   if ( cauldron->IsCalculationCoupled and cauldron->DoHighResDecompaction and not cauldron->isGeometricLoop ()) {
      status &= cauldron->setNodeDepths( currentTime );
   } else {
      status &= cauldron->calcNodeDepths( currentTime );
   }

   PETSC_ASSERT( status );
  
   status &= cauldron->Calculate_Pressure(  currentTime  );
   PETSC_ASSERT( status );

   Layer_Iterator basinLayers ( cauldron->layers, Descending, Basement_And_Sediments, Active_And_Inactive_Layers );

   while ( not basinLayers.Iteration_Is_Done ()) {
      currentLayer = basinLayers.Current_Layer ();
      currentLayer->setLayerElementActivity ( currentTime );
      basinLayers++;
   }


  return status;
}

bool MeshUnitTester::compareFiles ( const std::string& leftFileName,
                                    const std::string& rightFileName ) const {

   std::ifstream leftFile;
   std::ifstream rightFile;

   leftFile.open ( leftFileName.c_str ());

   if ( leftFile.fail ()) {
      std::cerr << " Could not open " << leftFileName << std::endl;
      return false;
   }

   rightFile.open ( rightFileName.c_str ());

   if ( rightFile.fail ()) {
      std::cerr << " Could not open " << rightFileName << std::endl;
      leftFile.close ();
      return false;
   }

   if ( leftFile == rightFile ) {
      std::cerr << " Files are the same." << std::endl;
      leftFile.close ();
      rightFile.close ();
      return false;
   }

   bool filesSame = true;
   int lineCount = 0;

   while ( not leftFile.eof () or not rightFile.eof ()) {
      ++lineCount;

      std::string leftLine;
      std::string rightLine;

      std::getline ( leftFile, leftLine );
      std::getline ( rightFile, rightLine );

      if ( leftLine != rightLine ) {
         std::cerr << " Lines " << lineCount << " of files "  << leftFileName << " and " << rightFileName << " are different." << std::endl;
         filesSame = false;
         break;
      }

   }

   if ( leftFile.eof () and not rightFile.eof ()) {
      std::cerr << leftFileName << "  is shorter." << std::endl;
      filesSame = false;
   } else if ( not leftFile.eof () and rightFile.eof ()) {
      std::cerr << rightFileName << "  is shorter." << std::endl;
      filesSame = false;
   }

   leftFile.close ();
   rightFile.close ();

   return filesSame;
}
