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
TEST(MixingPermeabilityArray, OneLithoHomogeneousNoAniso)
{


   std::vector<double> anisoVec ( { 1.0 });
   std::vector<double> percentVec ( { 100.0 } );

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



// Test mixing of three lithologies with homogeneous mixing and all have the same permeability
TEST(MixingPermeabilityArray, OneLithoHomogeneousWithAniso)
{


   std::vector<double> anisoVec ( { 3.0 });
   std::vector<double> percentVec ( { 100.0 } );

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

   // Fill perm for all lithos.
   for ( int i = 0; i < NumberOfValues; ++i, perm += delta ) {
      simplePermeabilities.getData ( 0 )[ i ] = perm;
      expectedPermeabilityNormal [ i ] = perm;
      expectedPermeabilityPlane [ i ] = perm * anisoVec [ 0 ];
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
