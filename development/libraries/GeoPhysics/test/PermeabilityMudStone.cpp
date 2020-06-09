//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/PermeabilityMudStone.h"
#include "ArrayDefinitions.h"
#include "AlignedMemoryAllocator.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>

#include <gtest/gtest.h>

using namespace GeoPhysics;

namespace {
const double epsilon = std::numeric_limits<double>::epsilon();
const double NaN = std::numeric_limits<double>::quiet_NaN();
}

// Input domain: PermeabilityMudStone::permeability
// permeabilityIncr = 1.5
// permeabilityDecr = 0.01
// depositionalPermeability = 0.01 mD (Shale, Marl, Dolo Mud, Lime Mud), 0.1 mD (chalk), 50 mD (siltstone)

// ves: invalid (-infinity, -1.0e+5) [1.0e+5, 0). valid = {0}, (0, 1.0e+5), [1.0e+5, infinity)
// maxVes: invalid = (-infinity, 0), {0}, [0, ves).  valid = [ves, infinity)
// calculatedPorosity: invalid = (-infinity, 0), (1, infinity). valid = {0}, (0, 1), {1}

TEST( PermeabilityMudStonePermeability, vesInvalid )
{
   // The weird/invalid cases
   // varying ves invalid classes
   EXPECT_DOUBLE_EQ( 3.91613803789194782345e-03, PermeabilityMudStone( 0.01, 1.5, 0.01).calculate( -1e+6, 1.0e+5, 0) );
   EXPECT_DOUBLE_EQ( 3.59572674388260737910e-02, PermeabilityMudStone( 0.1, 1.5, 0.01).calculate( -1e+5, 1.0e+5, 0.1) );
}

TEST( PermeabilityMudStonePermeability, maxvesInvalid )
{
   // varying maxVes invalid classes
   EXPECT_DOUBLE_EQ( 50.0 , PermeabilityMudStone( 50, 1.5, 0.01).calculate( 0, -1e+6, 1) );

   EXPECT_DOUBLE_EQ( 4.33392086020723751871e-02, PermeabilityMudStone( 0.05, 1.5, 0.01).calculate( 1e+4, 0, 0) );

   EXPECT_DOUBLE_EQ( 1.37050611171710742114e-02, PermeabilityMudStone( 0.5, 1.5, 0.01).calculate( 1e+6, 1e+3, 0.3) );
}

TEST( PermeabilityMudStonePermeability, calculatedPorosityInvalid )
{   // varying calculatedPorosity invalid classes
   EXPECT_DOUBLE_EQ( 5.0, PermeabilityMudStone( 5, 1.5, 0.01).calculate( 0,0, -5) );

   EXPECT_DOUBLE_EQ( 1.42269357739546418173e+01, PermeabilityMudStone( 40, 1.5, 0.01).calculate( 1e+4, 1e+5, 5) );
}

TEST( PermeabilityMudStonePermeability, validCases)
{   // the normal cases
   EXPECT_DOUBLE_EQ( 3.53553390593273810655e-03  , PermeabilityMudStone( 0.01, 1.5, 0.01).calculate( 1.0e+5, 1.0e+5, 0.0) );

   EXPECT_DOUBLE_EQ( 2.74101222343421484229e-03  , PermeabilityMudStone( 0.1, 1.5, 0.01).calculate( 1.0e+6, 1.0e+6, 0.2) );


   EXPECT_DOUBLE_EQ( 5.22936948520120403749e-01  , PermeabilityMudStone( 50, 1.5, 0.01).calculate( 1.0e+6, 2.0e+6, 0.3) );

   EXPECT_DOUBLE_EQ( 1000.0, PermeabilityMudStone( 6000, 1.5, 0.01).calculate( 1.0e+5, 1.0e+5, 0.6) );
}

//Input domain: PermeabilityMudStone :: permeabilityDerivative
// ves: invalid (-infinity, -1.0e+5) [1.0e+5, 0). valid = {0}, (0, 1.0e+5), [1.0e+5, infinity)
// maxVes: invalid = (-infinity, 0), {0}, [0, ves).  valid = [ves, infinity)
// calculatedPorosity: invalid = (-infinity, 0), (1, infinity). valid = {0}, (0, 1), {1}

   // The weird cases
TEST( PermeabilityMudStonePermeabilityDerivative, invalidLargeNegativeVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 0.01, 1.5, 0.01).calculateDerivative( -1e+6, 1.0e+5, 0, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ(3.91613803789194782345e-03, permeability);
   EXPECT_TRUE( std::isnan( derivative ));
}

TEST( PermeabilityMudStonePermeabilityDerivative, invalidSmallNegativeVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 0.05, 1.5, 0.01).calculateDerivative( -1e+5, 1.0e+5, 0.1, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 1.79786337194130368955e-02, permeability);
   EXPECT_TRUE( std::isinf( derivative) && derivative < 0.0 );
}

TEST( PermeabilityMudStonePermeabilityDerivative, invalidLargeNegativeMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 0.1, 1.5, 0.01).calculateDerivative( 0, -1e+6, 1, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 0.1, permeability );
   EXPECT_DOUBLE_EQ( -3.56012548899267987993e-09, derivative);
}

TEST( PermeabilityMudStonePermeabilityDerivative, invalidSmallVesSmallerMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 0.5, 1.5, 0.01).calculateDerivative( 1e+4, 0, 0, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 4.33392086020723710238e-01, permeability );
   EXPECT_DOUBLE_EQ( -1.61669724704030024964e-08, derivative);
}

TEST( PermeabilityMudStonePermeabilityDerivative, invalidNormalVesSmallerMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 1, 1.5, 0.01).calculateDerivative( 1e+6, 1e+3, 0.3, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ(  2.74101222343421484229e-02, permeability );
   EXPECT_DOUBLE_EQ( -3.73774394104665633438e-08, derivative);
}

TEST( PermeabilityMudStonePermeabilityDerivative, invalidNegativePorosity)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 2, 1.5, 0.01).calculateDerivative( 0, 0, -5, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 2.0, permeability );
   EXPECT_DOUBLE_EQ( -7.12025097798535975986e-08, derivative);
}

TEST( PermeabilityMudStonePermeabilityDerivative, invalidZeroPorosity)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 10, 1.5, 0.01).calculateDerivative( 1.0e+5, 1.0e+5, 0.0, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 3.53553390593273775266, permeability );
   EXPECT_DOUBLE_EQ( -2.65165042944955344439e-05, derivative);
}



// the normal cases
TEST( PermeabilityMudStonePermeabilityDerivative, lowVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 0.01, 1.5, 0.01).calculateDerivative( 1.0e+5, 1.0e+5, 0, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 3.53553390593273810655e-03, permeability );
   EXPECT_DOUBLE_EQ( -2.65165042944955341792e-08 , derivative );
}

TEST( PermeabilityMudStonePermeabilityDerivative, highVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 0.1, 1.5, 0.01).calculateDerivative( 1.0e+6, 1.0e+6, 0.2, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 2.74101222343421484229e-03, permeability );
   EXPECT_DOUBLE_EQ( -3.73774394104665699612e-09 , derivative );
}

TEST( PermeabilityMudStonePermeabilityDerivative, higherMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 50, 1.5, 0.01).calculateDerivative( 1.0e+6, 2.0e+6, 0.3, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 5.22936948520120403749e-01, permeability );
   EXPECT_DOUBLE_EQ( -4.75397225927382145993e-09 , derivative );
}

TEST( PermeabilityMudStonePermeabilityDerivative, highPermeability)
{
   double permeability = NaN, derivative = NaN;
   PermeabilityMudStone( 6000, 1.5, 0.01).calculateDerivative( 1.0e+5, 1.0e+5, 0.6, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ( 1000.0, permeability );
   EXPECT_DOUBLE_EQ( 0.0 , derivative );
}

TEST( PermeabilityMudStone , depoPerm )
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   ASSERT_DOUBLE_EQ( 0.01, PermeabilityMudStone( 0.01, 1.5, 0.01).depoPerm() );
}

TEST( PermeabilityMudStone, model)
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   ASSERT_EQ(MUDSTONE_PERMEABILITY, PermeabilityMudStone( 0.01, 1.5, 0.01).model() );
}




TEST ( PermeabilityMudStone, PermeabilityVector ) {

   PermeabilityMudStone mudstonePermeability ( 50, 1.5, 0.01 );

   const unsigned int Size = 20;
   ArrayDefs::Real_ptr ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   ArrayDefs::Real_ptr maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   ArrayDefs::Real_ptr porosity = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );
   ArrayDefs::Real_ptr permeability = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( Size );

   double NullValue = 99999.0;

   ves [ 0 ] = 0.0;
   maxVes [ 0 ] = 0.0;;

   for ( unsigned int i = 1; i < Size; ++i ) {
      ves [ i ] = ves [ i - 1 ] + ( i < Size / 2 ? 5.0e5 : -2.5e5 );
      maxVes [ i ] = maxVes [ i - 1 ] + 5.0e5;
   }

   double porosityStep = 0.5 / static_cast<double>(Size-1);

   for ( unsigned int i = 0; i < Size; ++i ) {
      porosity [ i ] = NullValue;//0.1 + static_cast<double>(i) * porosityStep;
   }

   mudstonePermeability.calculate ( Size, ves, maxVes, porosity, permeability );

   for ( unsigned int i = 0; i < Size; ++i ) {
      EXPECT_FLOAT_EQ ( mudstonePermeability.calculate( ves [ i ], maxVes [ i ], porosity [ i ] ), permeability [ i ] );
   }

}
