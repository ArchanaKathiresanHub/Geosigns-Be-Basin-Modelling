//
// Copyright (C) 2018 Shell International Exploration & Production.
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
#include "../../utilities/src/AlignedWorkSpaceArrays.h"
#include "../../utilities/src/AlignedMemoryAllocator.h"
#include "../../utilities/src/ConstantsMathematics.h"
#include "../src/PermeabilityMixer.h"
#include "../src/GeoPhysicalConstants.h"

#include <gtest/gtest.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

typedef formattingexception::GeneralException fastCauldronException;

using namespace GeoPhysics;
using Utilities::Maths::MilliDarcyToM2;

typedef AlignedWorkSpaceArrays<MaximumNumberOfLithologies> PermeabilityWorkSpaceArrays;


// Test mixing of three lithologies with homogeneous mixing and all have the same permeability
TEST(MixingPermeabilityArray, ThreeLithoHomogeneousNoAnisoSamePerm)
{


   std::vector<double> anisoVec ( { 1.0, 1.0, 1.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );

   PermeabilityMixer mixer;
   double layeringIndex;
   bool isFault = false;
   const int NumberOfValues = 8;

   // Layering Index = 1.0
   layeringIndex = 1.0;
   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::HOMOGENEOUS, isFault );



   PermeabilityWorkSpaceArrays simplePermeabilities ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityNormal   = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityPlane    = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeability = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );

   double perm = 1.0;
   double delta = 0.25;

   // Fill perm for all lithos.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 0 )[ i ] = perm;
      simplePermeabilities.getData ( 1 )[ i ] = perm;
      simplePermeabilities.getData ( 2 )[ i ] = perm;
      expectedPermeability [ i ] = perm;
   }

   mixer.mixPermeabilityArray ( NumberOfValues,
                                simplePermeabilities,
                                permeabilityNormal,
                                permeabilityPlane );

   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityNormal [ i ] / MilliDarcyToM2, expectedPermeability [ i ], 1.0e-12 );
   }

   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityPlane [ i ] / MilliDarcyToM2, expectedPermeability [ i ], 1.0e-12 );
   }

}


// Test mixing of three lithologies with homogeneous mixing and all have different permeability
TEST(MixingPermeabilityArray, ThreeLithoHomogeneousAniso )
{


   std::vector<double> anisoVec ( { 1.0, 2.0, 3.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );
   std::vector<double> fracVec;

   for ( size_t i = 0; i < percentVec.size (); ++i ) {
      fracVec.push_back ( 0.01 * percentVec [ i ]);
   }

   PermeabilityMixer mixer;
   double layeringIndex;
   bool isFault = false;
   const int NumberOfValues = 8;

   // Layering Index = 1.0
   layeringIndex = 1.0;
   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::HOMOGENEOUS, isFault );

   PermeabilityWorkSpaceArrays simplePermeabilities ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityNormal   = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityPlane    = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityNormal = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityPlane  = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );

   double perm = 1.0;
   double delta = 0.25;

   // Fill perm for litho 1.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 0 )[ i ] = perm;
   }

   // Fill perm for litho 2.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 1 )[ i ] = perm;
   }

   // Fill perm for litho 3.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 2 )[ i ] = perm;
   }

   // Compute expected permeability using expression from documentation
   for ( int i = 0; i < NumberOfValues; ++i ) {
      expectedPermeabilityNormal [ i ] = std::pow ( simplePermeabilities.getData ( 0 )[ i ], fracVec [ 0 ]) *
                                         std::pow ( simplePermeabilities.getData ( 1 )[ i ], fracVec [ 1 ]) *
                                         std::pow ( simplePermeabilities.getData ( 2 )[ i ], fracVec [ 2 ]);
      expectedPermeabilityPlane [ i ] = std::pow ( anisoVec [ 0 ] * simplePermeabilities.getData ( 0 )[ i ], fracVec [ 0 ]) *
                                        std::pow ( anisoVec [ 1 ] * simplePermeabilities.getData ( 1 )[ i ], fracVec [ 1 ]) *
                                        std::pow ( anisoVec [ 2 ] * simplePermeabilities.getData ( 2 )[ i ], fracVec [ 2 ]);
   }

   mixer.mixPermeabilityArray ( NumberOfValues,
                                simplePermeabilities,
                                permeabilityNormal,
                                permeabilityPlane );


   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityNormal [ i ] / MilliDarcyToM2, expectedPermeabilityNormal [ i ], 1.0e-12 );
   }

   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityPlane [ i ] / MilliDarcyToM2, expectedPermeabilityPlane [ i ], 1.0e-12 );
   }

}


// Test mixing of three lithologies with various layered mixing and different layering indices
TEST(MixingPermeabilityArray, ThreeLithoLayeredAniso )
{


   std::vector<double> anisoVec ( { 1.0, 2.0, 3.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );
   std::vector<double> fracVec;

   for ( size_t i = 0; i < percentVec.size (); ++i ) {
      fracVec.push_back ( 0.01 * percentVec [ i ]);
   }


   PermeabilityMixer mixer;
   bool isFault = false;
   const int NumberOfValues = 8;

   PermeabilityWorkSpaceArrays simplePermeabilities ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityNormal   = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityPlane    = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityNormal = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityPlane  = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );

   double perm = 1.0;
   double delta = 0.25;

   // Fill perm for litho 1.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 0 )[ i ] = perm;
   }

   // Fill perm for litho 2.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 1 )[ i ] = perm;
   }

   // Fill perm for litho 3.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 2 )[ i ] = perm;
   }

   const int NumberOfLayeringIndices = 4;
   // Should not include 0, or 0.25, these will be tested separately.
   double layeringIndices [ NumberOfLayeringIndices ] = {  0.1, 0.5, 0.75, 1.0 };

   for ( int t = 0; t < NumberOfLayeringIndices; ++t ) {
      mixer.reset ( percentVec, anisoVec, false, layeringIndices [ t ], DataAccess::Interface::LAYERED, isFault );

      double horizontalPower = ( 1.0 + 2.0 * layeringIndices [ t ]) / 3.0;
      double verticalPower   = ( 1.0 - 4.0 * layeringIndices [ t ]) / 3.0;

      // Compute expected permeability using expression from documentation
      for ( int i = 0; i < NumberOfValues; ++i ) {
         expectedPermeabilityNormal [ i ] = std::pow ( fracVec [ 0 ] * std::pow ( simplePermeabilities.getData ( 0 )[ i ], verticalPower ) +
                                                       fracVec [ 1 ] * std::pow ( simplePermeabilities.getData ( 1 )[ i ], verticalPower ) +
                                                       fracVec [ 2 ] * std::pow ( simplePermeabilities.getData ( 2 )[ i ], verticalPower ), 1.0 / verticalPower );

         expectedPermeabilityPlane [ i ] = std::pow ( fracVec [ 0 ] * std::pow ( anisoVec [ 0 ] * simplePermeabilities.getData ( 0 )[ i ], horizontalPower ) +
                                                      fracVec [ 1 ] * std::pow ( anisoVec [ 1 ] * simplePermeabilities.getData ( 1 )[ i ], horizontalPower ) +
                                                      fracVec [ 2 ] * std::pow ( anisoVec [ 2 ] * simplePermeabilities.getData ( 2 )[ i ], horizontalPower ), 1.0 / horizontalPower );
      }

      mixer.mixPermeabilityArray ( NumberOfValues,
                                   simplePermeabilities,
                                   permeabilityNormal,
                                   permeabilityPlane );


      for ( int i = 0; i < NumberOfValues; ++i ) {
         EXPECT_NEAR ( permeabilityNormal [ i ] / MilliDarcyToM2, expectedPermeabilityNormal [ i ], 1.0e-12 );
      }

      for ( int i = 0; i < NumberOfValues; ++i ) {
         EXPECT_NEAR ( permeabilityPlane [ i ] / MilliDarcyToM2, expectedPermeabilityPlane [ i ], 1.0e-12 );
      }

   }

}


// Test mixing of three lithologies with various layered mixing and layering inded of 0.25, this is a special case.
TEST(MixingPermeabilityArray, ThreeLithoLayeredAniso_LI_0_25 )
{

   std::vector<double> anisoVec ( { 1.0, 2.0, 3.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );
   std::vector<double> fracVec;

   for ( size_t i = 0; i < percentVec.size (); ++i ) {
      fracVec.push_back ( 0.01 * percentVec [ i ]);
   }


   PermeabilityMixer mixer;
   bool isFault = false;
   const int NumberOfValues = 8;

   PermeabilityWorkSpaceArrays simplePermeabilities ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityNormal   = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityPlane    = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityNormal = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityPlane  = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );

   double perm = 1.0;
   double delta = 0.25;

   // Fill perm for litho 1.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 0 )[ i ] = perm;
   }

   // Fill perm for litho 2.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 1 )[ i ] = perm;
   }

   // Fill perm for litho 3.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 2 )[ i ] = perm;
   }

   double layeringIndex = 0.25;

   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );

   double horizontalPower = ( 1.0 + 2.0 * layeringIndex ) / 3.0; // = 0.5
   double verticalPower   = ( 1.0 - 4.0 * layeringIndex ) / 3.0; // = 0

   // Compute expected permeability using expression from documentation
   for ( int i = 0; i < NumberOfValues; ++i ) {
      expectedPermeabilityNormal [ i ] = std::exp ( fracVec [ 0 ] * std::log ( simplePermeabilities.getData ( 0 )[ i ] ) +
                                                    fracVec [ 1 ] * std::log ( simplePermeabilities.getData ( 1 )[ i ] ) +
                                                    fracVec [ 2 ] * std::log ( simplePermeabilities.getData ( 2 )[ i ] ));

      expectedPermeabilityPlane [ i ] = std::pow ( fracVec [ 0 ] * std::pow ( anisoVec [ 0 ] * simplePermeabilities.getData ( 0 )[ i ], horizontalPower ) +
                                                   fracVec [ 1 ] * std::pow ( anisoVec [ 1 ] * simplePermeabilities.getData ( 1 )[ i ], horizontalPower ) +
                                                   fracVec [ 2 ] * std::pow ( anisoVec [ 2 ] * simplePermeabilities.getData ( 2 )[ i ], horizontalPower ), 1.0 / horizontalPower );
   }

   mixer.mixPermeabilityArray ( NumberOfValues,
                                simplePermeabilities,
                                permeabilityNormal,
                                permeabilityPlane );


   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityNormal [ i ] / MilliDarcyToM2, expectedPermeabilityNormal [ i ], 1.0e-12 );
   }

   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityPlane [ i ] / MilliDarcyToM2, expectedPermeabilityPlane [ i ], 1.0e-12 );
   }

}

// Test mixing of three lithologies with various layered mixing and layering inded of 0, this is a special case.
TEST(MixingPermeabilityArray, ThreeLithoLayeredAniso_LI_0 )
{


   std::vector<double> anisoVec ( { 1.0, 2.0, 3.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );
   std::vector<double> fracVec;

   for ( size_t i = 0; i < percentVec.size (); ++i ) {
      fracVec.push_back ( 0.01 * percentVec [ i ]);
   }


   PermeabilityMixer mixer;
   bool isFault = false;
   const int NumberOfValues = 8;

   PermeabilityWorkSpaceArrays simplePermeabilities ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityNormal   = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         permeabilityPlane    = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityNormal = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );
   ArrayDefs::Real_ptr         expectedPermeabilityPlane  = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( NumberOfValues );

   double perm = 1.0;
   double delta = 0.25;

   // Fill perm for litho 1.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 0 )[ i ] = perm;
   }

   // Fill perm for litho 2.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 1 )[ i ] = perm;
   }

   // Fill perm for litho 3.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 2 )[ i ] = perm;
   }

   double layeringIndex = 0;

   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );

   double horizontalPower = ( 1.0 + 2.0 * layeringIndex ) / 3.0; // = 1/3
   double verticalPower   = ( 1.0 - 4.0 * layeringIndex ) / 3.0; // = 1/3

   // Compute expected permeability using expression from documentation
   for ( int i = 0; i < NumberOfValues; ++i ) {
      expectedPermeabilityNormal [ i ] = std::pow ( fracVec [ 0 ] * std::pow ( simplePermeabilities.getData ( 0 )[ i ], verticalPower ) +
                                                    fracVec [ 1 ] * std::pow ( simplePermeabilities.getData ( 1 )[ i ], verticalPower ) +
                                                    fracVec [ 2 ] * std::pow ( simplePermeabilities.getData ( 2 )[ i ], verticalPower ), 1.0 / verticalPower );

      expectedPermeabilityPlane [ i ] = std::pow ( fracVec [ 0 ] * std::pow ( anisoVec [ 0 ] * simplePermeabilities.getData ( 0 )[ i ], horizontalPower ) +
                                                   fracVec [ 1 ] * std::pow ( anisoVec [ 1 ] * simplePermeabilities.getData ( 1 )[ i ], horizontalPower ) +
                                                   fracVec [ 2 ] * std::pow ( anisoVec [ 2 ] * simplePermeabilities.getData ( 2 )[ i ], horizontalPower ), 1.0 / horizontalPower );
   }

   mixer.mixPermeabilityArray ( NumberOfValues,
                                simplePermeabilities,
                                permeabilityNormal,
                                permeabilityPlane );


   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityNormal [ i ] / MilliDarcyToM2, expectedPermeabilityNormal [ i ], 1.0e-12 );
   }

   for ( int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_NEAR ( permeabilityPlane [ i ] / MilliDarcyToM2, expectedPermeabilityPlane [ i ], 1.0e-12 );
   }

}
