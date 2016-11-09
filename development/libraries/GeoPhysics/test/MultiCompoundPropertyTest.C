//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/MultiCompoundProperty.h"
#include "ArrayDefinitions.h"
#include <gtest/gtest.h>


// Test that the addresses of each of the simple and mixed data are correctly aligned
// if the number of values ensures alignment
TEST ( MultiCompoundPropertyTest, AlignedTestAddress ) {

   const unsigned int NumberOfLithologies = 2;
   const unsigned int NumberOfValues = 4 * ARRAY_ALIGNMENT / sizeof (double);

   GeoPhysics::MultiCompoundProperty mcp ( NumberOfLithologies, NumberOfValues );

   uintptr_t arrayAddress;

   //--------------------------------

   // Check that the size of the multi-component property object is correct
   EXPECT_EQ ( mcp.getNumberOfLithologies (), NumberOfLithologies );
   EXPECT_EQ ( mcp.getNumberOfValues (), NumberOfValues );
   EXPECT_EQ ( mcp.getLeadingDimension (), NumberOfValues );

   //--------------------------------

   // Now check that the addresses are correctly aligned in memory
   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 0 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 1 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getMixedData ());
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   //--------------------------------

   // Finally check that the pointers are not the same for each of the selections.
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getSimpleData ( 1 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getMixedData (), false );
   EXPECT_EQ ( mcp.getSimpleData ( 1 ) == mcp.getMixedData (), false );
}


// Test that the addresses of each of the simple and mixed data are correctly aligned
// if the number of values ensures no alignment
TEST ( MultiCompoundPropertyTest, UnalignedTestAddress ) {

   const unsigned int NumberOfLithologies = 3;
   const unsigned int NumberOfValues = 4 * ARRAY_ALIGNMENT / sizeof (double) + 3;
   const unsigned int ExpectedAlignment = 5 * ARRAY_ALIGNMENT / sizeof (double);

   GeoPhysics::MultiCompoundProperty mcp ( NumberOfLithologies, NumberOfValues );

   uintptr_t arrayAddress;

   //--------------------------------

   // Check that the size of the multi-component property object is correct
   EXPECT_EQ ( mcp.getNumberOfLithologies (), NumberOfLithologies );
   EXPECT_EQ ( mcp.getNumberOfValues (), NumberOfValues );
   EXPECT_EQ ( mcp.getLeadingDimension (), ExpectedAlignment );

   //--------------------------------

   // Now check that the addresses are correctly aligned in memory
   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 0 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 1 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 2 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getMixedData ());
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   //--------------------------------

   // Finally check that the pointers are not the same for each of the selections.
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getSimpleData ( 1 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getSimpleData ( 2 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 1 ) == mcp.getSimpleData ( 2 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getMixedData (), false );
   EXPECT_EQ ( mcp.getSimpleData ( 1 ) == mcp.getMixedData (), false );
   EXPECT_EQ ( mcp.getSimpleData ( 2 ) == mcp.getMixedData (), false );

}


// Test that the values are stored correctly to each of the simple and mixed data
// if the number of values ensures alignment
TEST ( MultiCompoundPropertyTest, AlignedTestValues ) {

   const unsigned int NumberOfLithologies = 2;
   const unsigned int NumberOfValues = 4 * ARRAY_ALIGNMENT / sizeof (double);

   GeoPhysics::MultiCompoundProperty mcp ( NumberOfLithologies, NumberOfValues );
   double value = 0.0;

   //------------------------------
   // Assign values to the multi-component property

   for ( unsigned int litho = 0; litho < NumberOfLithologies; ++litho ) {

      for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
         mcp.getSimpleData ( litho, i ) = value;
         value += 1.0;
      }

   }

   for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
      mcp.getMixedData ( i ) = value;
      value += 1.0;
   }

   //------------------------------
   // Compare the values obtained from the multi-component property to the expected value
   // Values are obtained using both the accessor methods

   value = 0.0;

   for ( unsigned int litho = 0; litho < NumberOfLithologies; ++litho ) {

      for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
         EXPECT_EQ ( mcp.getSimpleData ( litho, i ), value );
         EXPECT_EQ ( mcp.getSimpleData ( litho )[ i ], value );
         value += 1.0;
      }

   }

   for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_EQ ( mcp.getMixedData ( i ), value );
      EXPECT_EQ ( mcp.getMixedData ()[ i ], value );
      value += 1.0;
   }

}


// Test that the values are stored correctly to each of the simple and mixed data
// if the number of values ensures no alignment
TEST ( MultiCompoundPropertyTest, UnalignedTestValues ) {

   const unsigned int NumberOfLithologies = 2;
   const unsigned int NumberOfValues = 4 * ARRAY_ALIGNMENT / sizeof (double) + 3;

   GeoPhysics::MultiCompoundProperty mcp ( NumberOfLithologies, NumberOfValues );
   double value = 0.0;

   //------------------------------
   // Assign values to the multi-component property

   for ( unsigned int litho = 0; litho < NumberOfLithologies; ++litho ) {

      for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
         mcp.getSimpleData ( litho, i ) = value;
         value += 1.0;
      }

   }

   for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
      mcp.getMixedData ( i ) = value;
      value += 1.0;
   }

   //------------------------------
   // Compare the values obtained from the multi-component property to the expected value
   // Values are obtained using both the accessor methods

   value = 0.0;

   for ( unsigned int litho = 0; litho < NumberOfLithologies; ++litho ) {

      for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
         EXPECT_EQ ( mcp.getSimpleData ( litho, i ), value );
         EXPECT_EQ ( mcp.getSimpleData ( litho )[ i ], value );
         value += 1.0;
      }

   }

   for ( unsigned int i = 0; i < NumberOfValues; ++i ) {
      EXPECT_EQ ( mcp.getMixedData ( i ), value );
      EXPECT_EQ ( mcp.getMixedData ()[ i ], value );
      value += 1.0;
   }

}


// Test that the addresses of each of the simple and mixed data are correctly aligned
// before and after a resizing. The number of values ensures first alignment and then not.
TEST ( MultiCompoundPropertyTest, Aligned2UnalignedTestAddress ) {

   const unsigned int FirstNumberOfLithologies = 2;
   const unsigned int FirstNumberOfValues = 4 * ARRAY_ALIGNMENT / sizeof (double);
   const unsigned int FirstExpectedAlignedSize = 4 * ARRAY_ALIGNMENT / sizeof (double);

   const unsigned int SecondNumberOfLithologies = 3;
   const unsigned int SecondNumberOfValues = 4 * ARRAY_ALIGNMENT / sizeof (double) + 1;
   const unsigned int SecondExpectedAlignedSize = 5 * ARRAY_ALIGNMENT / sizeof (double);

   GeoPhysics::MultiCompoundProperty mcp ( FirstNumberOfLithologies, FirstNumberOfValues );

   uintptr_t arrayAddress;

   //--------------------------------

   // Check that the size of the multi-component property object is correct
   EXPECT_EQ ( mcp.getNumberOfLithologies (), FirstNumberOfLithologies );
   EXPECT_EQ ( mcp.getNumberOfValues (), FirstNumberOfValues );
   EXPECT_EQ ( mcp.getLeadingDimension (), FirstExpectedAlignedSize );

   //--------------------------------

   // Now check that the addresses are correctly aligned in memory
   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 0 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 1 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getMixedData ());
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   //--------------------------------

   // Finally check that the pointers are not the same for each of the selections.
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getSimpleData ( 1 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getMixedData (), false );
   EXPECT_EQ ( mcp.getSimpleData ( 1 ) == mcp.getMixedData (), false );

   //--------------------------------

   mcp.resize ( SecondNumberOfLithologies, SecondNumberOfValues );

   // Check that the size of the multi-component property object is correct
   EXPECT_EQ ( mcp.getNumberOfLithologies (), SecondNumberOfLithologies );
   EXPECT_EQ ( mcp.getNumberOfValues (), SecondNumberOfValues );
   EXPECT_EQ ( mcp.getLeadingDimension (), SecondExpectedAlignedSize );

   //--------------------------------

   // Now check that the addresses are correctly aligned in memory
   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 0 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 1 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 2 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getMixedData ());
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   //--------------------------------

   // Finally check that the pointers are not the same for each of the selections.
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getSimpleData ( 1 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getSimpleData ( 2 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 1 ) == mcp.getSimpleData ( 2 ), false );

   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getMixedData (), false );
   EXPECT_EQ ( mcp.getSimpleData ( 1 ) == mcp.getMixedData (), false );
   EXPECT_EQ ( mcp.getSimpleData ( 2 ) == mcp.getMixedData (), false );

}


// Test that the addresses of each of the simple and mixed data are correctly aligned
// before and after a resizing. The number of values ensures first no alignment and then does align correctly.
TEST ( MultiCompoundPropertyTest, Unaligned2AlignedTestAddress ) {

   const unsigned int FirstNumberOfLithologies = 2;
   const unsigned int FirstNumberOfValues = 4 * ARRAY_ALIGNMENT / sizeof (double) + 1;

   const unsigned int SecondNumberOfLithologies = 1;
   const unsigned int SecondNumberOfValues = 3 * ARRAY_ALIGNMENT / sizeof (double);

   GeoPhysics::MultiCompoundProperty mcp ( FirstNumberOfLithologies, FirstNumberOfValues );

   uintptr_t arrayAddress;

   //--------------------------------

   // Check that the size of the multi-component property object is correct
   EXPECT_EQ ( mcp.getNumberOfLithologies (), FirstNumberOfLithologies );
   EXPECT_EQ ( mcp.getNumberOfValues (), FirstNumberOfValues );
   // EXPECT_EQ ( mcp.getLeadingDimension (), NumberOfValues );

   //--------------------------------

   // Now check that the addresses are correctly aligned in memory
   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 0 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 1 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getMixedData ());
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   //--------------------------------

   // Finally check that the pointers are not the same for each of the selections.
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getSimpleData ( 1 ), false );
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getMixedData (), false );
   EXPECT_EQ ( mcp.getSimpleData ( 1 ) == mcp.getMixedData (), false );

   //--------------------------------

   mcp.resize ( SecondNumberOfLithologies, SecondNumberOfValues );

   // Check that the size of the multi-component property object is correct
   EXPECT_EQ ( mcp.getNumberOfLithologies (), SecondNumberOfLithologies );
   EXPECT_EQ ( mcp.getNumberOfValues (), SecondNumberOfValues );

   //--------------------------------

   // Now check that the addresses are correctly aligned in memory
   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getSimpleData ( 0 ));
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   arrayAddress = reinterpret_cast<uintptr_t>(mcp.getMixedData ());
   EXPECT_EQ ( arrayAddress % ARRAY_ALIGNMENT, 0 );

   //--------------------------------

   // Finally check that the pointers are not the same for each of the selections.
   EXPECT_EQ ( mcp.getSimpleData ( 0 ) == mcp.getMixedData (), false );

}
