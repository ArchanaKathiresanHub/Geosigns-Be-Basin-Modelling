//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/CompoundLithology.h"
#include "../src/CompoundLithologyComposition.h"
#include "../src/GeoPhysicsProjectHandle.h"
#include "../src/GeoPhysicsObjectFactory.h"
#include "../src/LithologyManager.h"
#include "../src/MultiCompoundProperty.h"
#include "../src/CompoundProperty.h"
#include "../../utilities/src/FormattingException.h"
#include "../src/PermeabilityMixer.h"

#include <gtest/gtest.h>
#include <stdlib.h>

typedef formattingexception::GeneralException fastCauldronException;

using namespace GeoPhysics;

TEST(MixingPermeability, multiTest)
{

   GeoPhysics::ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   GeoPhysics::ProjectHandle* projectHandle = dynamic_cast< GeoPhysics::ProjectHandle* >(DataAccess::Interface::OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   int NumberOflithologies;
   int NumberOfIterations;
   CompoundLithologyComposition composition;

   const unsigned int Size = 100;

   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* porosity = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* chemicalCompaction = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* porosityDerivative = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* permeabilityNormal = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* permeabilityPlane = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* permeabilityDerivative = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   CompoundProperty* mcps  = new CompoundProperty [ Size ];

   PermeabilityMixer::PermeabilityWorkSpaceArrays workSpace ( Size );

   double h = 1.0e7 / static_cast<double>(Size - 1);
   double v = 0.0;
   double mv = 0.0;

   for ( int i = 0; i < Size; ++i, mv += h ) {
      ves [ i ] = mv;
      maxVes [ i ] = mv;
      chemicalCompaction  [ i ] = 0.0;
   }

   for ( unsigned int whichLitho = 1; whichLitho <= 7; ++whichLitho ) {

      switch ( whichLitho ) {
        case 1: composition.setComposition ( "SM.Mudst.40%Clay" ,                 "",                 "", 100.0,  0.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 1; break;
        case 2: composition.setComposition ( "SM. Sandstone"    , "SM.Mudst.50%Clay", "SM.Mudst.60%Clay",  33.0, 33.0, 34.0, "Layered", 0.0 ); NumberOflithologies = 3; break;
        case 3: composition.setComposition ( "SM. Sandstone"    , "SM.Mudst.40%Clay", "SM.Mudst.50%Clay",  33.0, 33.0, 34.0, "Layered", 0.0 ); NumberOflithologies = 3; break;
        case 4: composition.setComposition ( "Std. Siltstone"   ,   "Std. Sandstone",       "Std. Shale",  33.0, 33.0, 34.0, "Layered", 0.0 ); NumberOflithologies = 3; break;
        case 5: composition.setComposition ( "SM.Mudst.60%Clay" ,                 "",                 "", 100.0,  0.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 1; break;
        case 6: composition.setComposition ( "SM.Mudst.50%Clay" , "SM.Mudst.40%Clay",                 "",  67.0, 33.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 2; break;
        case 7: composition.setComposition ( "Std. Sandstone"   ,   "Std. Sandstone",                 "",  67.0, 33.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 2; break;

      default :
         throw fastCauldronException () << " Incorrect lithology selection";
      }

      CompoundLithology* myLitho = projectHandle->getLithologyManager ().getCompoundLithology ( composition );
      MultiCompoundProperty mcp ( NumberOflithologies, Size );

      for ( unsigned int j = 0; j < Size; ++j ) {
         myLitho->getPorosity ( ves [ j ], maxVes [ j ], false, chemicalCompaction [ j ], mcps [ j ]);

         mcp.getSimpleData ( 0, j ) = mcps [ j ] ( 0 );

         if ( NumberOflithologies > 1 ) {
            mcp.getSimpleData ( 1, j ) = mcps [ j ] ( 1 );
         }

         if ( NumberOflithologies > 2 ) {
            mcp.getSimpleData ( 2, j ) = mcps [ j ] ( 2 );
         }

         mcp.getMixedData ( j ) = mcps [ j ].mixedProperty ();
      }

      myLitho->calcBulkPermeabilityNP ( Size, ves, maxVes, mcp, permeabilityNormal, permeabilityPlane, workSpace );

      for ( unsigned int i = 0; i < Size; ++i ) {
         double permeabilityNormalScalar;
         double permeabilityPlaneScalar;

         myLitho->calcBulkPermeabilityNP ( ves [ i ], maxVes [ i ], mcps [ i ], permeabilityNormalScalar, permeabilityPlaneScalar );
         EXPECT_NEAR ( permeabilityNormalScalar, permeabilityNormal [ i ], 1.0e-12 );
         EXPECT_NEAR ( permeabilityPlaneScalar,  permeabilityPlane  [ i ], 1.0e-12 );
      }

   }

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( porosity );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( chemicalCompaction );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( porosityDerivative );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( permeabilityNormal );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( permeabilityPlane );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( permeabilityDerivative );
   delete [] mcps;
}
