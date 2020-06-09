//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/PermeabilityMultiPoint.h"
#include "../src/PermeabilityMudStone.h"
#include "../src/PermeabilitySandStone.h"
#include "../src/Permeability.h"

#include "AlignedMemoryAllocator.h"
#include "ArrayDefinitions.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>
#include <chrono>

#include <gtest/gtest.h>

using namespace GeoPhysics;

namespace {

const double sm_mudst_20_porositySamples[2] = { 0.05, 0.6 };
const double sm_mudst_20_permeabilitySamples[2] = { -3.0e+00, 10.0 };
const double sm_mudst_20_surfacePorosity = 0.5632;

const double sm_mudst_60_porositySamples[6] = { 0.05, 0.2, 0.3, 0.4, 0.5, 0.6 };
const double sm_mudst_60_permeabilitySamples[6] = { -5.93e+00, -5.23e+00, -4.67e+00, -3.9e+00, -3.0e+00, -2.0e+00 };
const double sm_mudst_60_surfacePorosity = 0.6685;

const std::vector<double> porositySamples_20( sm_mudst_20_porositySamples, sm_mudst_20_porositySamples + 2 );
const std::vector<double> permeabilitySamples_20( sm_mudst_20_permeabilitySamples, sm_mudst_20_permeabilitySamples + 2 );

const std::vector<double> porositySamples_60( sm_mudst_60_porositySamples, sm_mudst_60_porositySamples + 6 );
const std::vector<double> permeabilitySamples_60( sm_mudst_60_permeabilitySamples, sm_mudst_60_permeabilitySamples + 6 );

}

TEST( PermeabilityMultiPointArrayTest, SinglePanelTest ) {

   PermeabilityMultiPoint mpp ( sm_mudst_20_surfacePorosity, porositySamples_20, permeabilitySamples_20 );

   const unsigned int Size = 100;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* porosity = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* permeability = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );

   double h = 1.0e7 / static_cast<double>(Size - 1);
   double v = 0.0;

   for ( int i = 0; i < Size; ++i, v += h ) {
      ves [ i ] = v;
      maxVes [ i ] = v;
   }

   double maxPorosity = sm_mudst_20_surfacePorosity;

   h = ( maxPorosity - 0.001 ) / static_cast<double>(Size - 1);
   v = maxPorosity;

   for ( int i = 0; i < Size; ++i, v += h ) {
      porosity [ i ] = v;
   }

   mpp.calculate ( Size, ves, maxVes, porosity, permeability );

   for ( int j = 0; j < Size; ++j ) {
      EXPECT_NEAR ( mpp.calculate ( ves [ j ], maxVes [ j ], porosity [ j ] ), permeability [ j ], 1.0e-12 );
   }

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( porosity );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( permeability );
}

TEST( PermeabilityMultiPointArrayTest, MultiPanelTest ) {

   PermeabilityMultiPoint mpp ( sm_mudst_60_surfacePorosity, porositySamples_60, permeabilitySamples_60 );

   const unsigned int Size = 100;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* porosity = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   double* permeability = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );

   double h = 1.0e7 / static_cast<double>(Size - 1);
   double v = 0.0;

   for ( int i = 0; i < Size; ++i, v += h ) {
      ves [ i ] = v;
      maxVes [ i ] = v;
   }

   double maxPorosity = sm_mudst_60_surfacePorosity;

   h = ( maxPorosity - 0.001 ) / static_cast<double>(Size - 1);
   v = maxPorosity;

   for ( int i = 0; i < Size; ++i, v += h ) {
      porosity [ i ] = v;
   }

   mpp.calculate ( Size, ves, maxVes, porosity, permeability );

   for ( int j = 0; j < Size; ++j ) {
      EXPECT_NEAR ( mpp.calculate ( ves [ j ], maxVes [ j ], porosity [ j ] ), permeability [ j ], 1.0e-12 );
   }

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( porosity );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free ( permeability );

}
