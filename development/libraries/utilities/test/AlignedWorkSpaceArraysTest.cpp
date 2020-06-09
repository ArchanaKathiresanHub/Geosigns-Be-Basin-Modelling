
#include "../src/AlignedWorkSpaceArrays.h"

#include <gtest/gtest.h>


// Test alignment or workspace vectors
TEST ( AlignedWorkSpaceArraysTest, AlignmentTest01 )
{
   const unsigned int MaximumNumberOfVectors = 3;
   const unsigned int NumberOfValues = 100;
   const unsigned int ExpectedLeadingDimension = 100;

   AlignedWorkSpaceArrays<MaximumNumberOfVectors> workSpace ( NumberOfValues );

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {
      EXPECT_EQ ( reinterpret_cast<uintptr_t>(workSpace.getData ( i )) % ARRAY_ALIGNMENT == 0, true );
   }

   EXPECT_EQ ( workSpace.getLeadingDimension (), ExpectedLeadingDimension );
   EXPECT_EQ ( workSpace.getNumberOfValues (), NumberOfValues );

}


// Test alignment or workspace vectors
TEST ( AlignedWorkSpaceArraysTest, AlignmentTest02 )
{
   const unsigned int MaximumNumberOfVectors = 5;
   const unsigned int NumberOfValues = 53;
   const unsigned int ExpectedLeadingDimension = 56; // = 53 + 4 - 53 rem 4

   AlignedWorkSpaceArrays<MaximumNumberOfVectors> workSpace ( NumberOfValues );

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {
      EXPECT_EQ ( reinterpret_cast<uintptr_t>(workSpace.getData ( i )) % ARRAY_ALIGNMENT == 0, true );
   }

   EXPECT_EQ ( workSpace.getLeadingDimension (), ExpectedLeadingDimension );
   EXPECT_EQ ( workSpace.getNumberOfValues (), NumberOfValues );
}


// Test alignment or workspace vectors
TEST ( AlignedWorkSpaceArraysTest, ValueTest01 )
{
   const unsigned int MaximumNumberOfVectors = 3;
   const unsigned int NumberOfValues = 53;

   AlignedWorkSpaceArrays<MaximumNumberOfVectors> workSpace ( NumberOfValues );

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {
      EXPECT_EQ ( reinterpret_cast<uintptr_t>(workSpace.getData ( i )) % ARRAY_ALIGNMENT == 0, true );
   }

   EXPECT_EQ ( workSpace.getNumberOfValues (), NumberOfValues );

   double value = 0.0;

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {

      for ( unsigned int j = 0; j < NumberOfValues; ++j, value += 1.0 ) {
         workSpace.getData ( i )[ j ] = value;
      }

   }

   value = 0.0;

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {

      for ( unsigned int j = 0; j < NumberOfValues; ++j, value += 1.0 ) {
         EXPECT_EQ ( workSpace.getData ( i )[ j ], value );
      }

   }

}

// Test alignment or workspace vectors
TEST ( AlignedWorkSpaceArraysTest, ResizeTest01 )
{
   const unsigned int MaximumNumberOfVectors = 3;
   const unsigned int FirstNumberOfValues = 23;
   const unsigned int SecondNumberOfValues = 71;

   AlignedWorkSpaceArrays<MaximumNumberOfVectors> workSpace ( FirstNumberOfValues );

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {
      EXPECT_EQ ( reinterpret_cast<uintptr_t>(workSpace.getData ( i )) % ARRAY_ALIGNMENT == 0, true );
   }

   EXPECT_EQ ( workSpace.getNumberOfValues (), FirstNumberOfValues );

   workSpace.resize ( SecondNumberOfValues );

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {
      EXPECT_EQ ( reinterpret_cast<uintptr_t>(workSpace.getData ( i )) % ARRAY_ALIGNMENT == 0, true );
   }

   EXPECT_EQ ( workSpace.getNumberOfValues (), SecondNumberOfValues );

   double value = 0.0;

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {

      for ( unsigned int j = 0; j < SecondNumberOfValues; ++j, value += 1.0 ) {
         workSpace.getData ( i )[ j ] = value;
      }

   }

   value = 0.0;

   for ( unsigned int i = 0; i < MaximumNumberOfVectors; ++i ) {

      for ( unsigned int j = 0; j < SecondNumberOfValues; ++j, value += 1.0 ) {
         EXPECT_EQ ( workSpace.getData ( i )[ j ], value );
      }

   }

}
