#include "MultiDimRange.h"
#include <iostream>

#include <gtest/gtest.h>

// Note:
// The implementation of the class MultiDimRange is so simple, that only
// testing the functions as a unit has no value. Here two cases are tested
// single-pass and multi-pass traversal of the range.

   // test case 1: null-set
TEST( MultiDimRangeSinglePass, EmptySet )
{  int count = 0;
   std::vector<ScalarRange> empty;
   MultiDimRange r( empty  );

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;

   ASSERT_EQ( 0,  count );
}

// test case 2: one dimension: one empty range
TEST( MultiDimRangeSinglePass, OneEmptyRange)
{  int count = 0;
   std::vector<ScalarRange> oneDimension(1, ScalarRange(0, -1, 0.125));
   MultiDimRange r( oneDimension );

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;

   ASSERT_EQ( 0,  count );
}

// test case 2: one dimension: one non-empty range
TEST( MultiDimRangeSinglePass, OneNonEmptyRange)
{  int count = 0;
   std::vector<ScalarRange> oneDimension(1, ScalarRange(0, 1, 0.125));
   MultiDimRange r( oneDimension );

   for ( ; ! r.isPastEnd(); r.nextValue() )
   {
      std::vector<double> values;
      r.getValue(values);
      ASSERT_EQ( 1,  values.size() );
      ASSERT_EQ( static_cast<double>(count) / 8 ,  values[0] );
      ++count;
   }

   ASSERT_EQ( 9,  count );
}


// test case 3: two dimensions: one empty range, one non-empty
TEST( MultiDimRangeSinglePass, OneEmptyRangeAndOneNonEmpty )
{  int count = 0;
   std::vector<ScalarRange> twoDim;
   twoDim.push_back( ScalarRange(0, 1, 0.125));
   twoDim.push_back( ScalarRange(0, -1, 0.125));
   MultiDimRange r( twoDim );

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;

   ASSERT_EQ( 0,  count );
}

// test case 4: two dimensions: two non-empty ranges
TEST( MultiDimRangeSinglePass, TwoNonEmptyRanges )
{  int count = 0;
   std::vector<ScalarRange> twoDim;
   twoDim.push_back( ScalarRange(0, 1, 0.125));
   twoDim.push_back( ScalarRange(0, 2, 0.125));
   MultiDimRange r( twoDim );

   for ( ; ! r.isPastEnd(); r.nextValue() )
   {
      std::vector<double> values;
      r.getValue(values);

      ASSERT_EQ( 2, values.size() );
      ASSERT_EQ( static_cast<double>(count % 9) / 8,  values[0] );
      ASSERT_EQ( static_cast<double>(count / 9) / 8,  values[1] );

      ++count;
   }

   ASSERT_EQ( 9*17,  count );
}

}

// test case 1: null-set
TEST( MultiDimRangeMultiPass, EmptySet )
{  int count = 0;
   std::vector<ScalarRange> empty;
   MultiDimRange r( empty  );

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;
   ASSERT_EQ( 0,  count );

   r.reset();

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;

   ASSERT_EQ( 0,  count );
}

// test case 2: one dimension: one empty range
TEST( MultiDimRangeMultiPass, OneEmptyRange )
{  int count = 0;
   std::vector<ScalarRange> oneDimension(1, ScalarRange(0, -1, 0.125));
   MultiDimRange r( oneDimension );

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;
   ASSERT_EQ( 0,  count );

   r.reset();

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;

   ASSERT_EQ( 0,  count );
}

// test case 2: one dimension: one non-empty range
TEST( MultiDimRangeMultiPass, OneNonEmptyRange)
{  int count = 0;
   std::vector<ScalarRange> oneDimension(1, ScalarRange(0, 1, 0.125));
   MultiDimRange r( oneDimension );

   for ( ; ! r.isPastEnd(); r.nextValue() )
   {
      std::vector<double> values;
      r.getValue(values);
      ASSERT_EQ( 1,  values.size() );
      ASSERT_EQ( static_cast<double>(count) / 8 ,  values[0] );
      ++count;
   }

   ASSERT_EQ( 9,  count );

   r.reset();
   count = 0;

   for ( ; ! r.isPastEnd(); r.nextValue() )
   {
      std::vector<double> values;
      r.getValue(values);
      ASSERT_EQ( 1,  values.size() );
      ASSERT_EQ( static_cast<double>(count) / 8 ,  values[0] );
      ++count;
   }
}


// test case 3: two dimensions: one empty range, one non-empty
TEST( MultiDimRangeMultiPass, OneEmptyRangeAndOneNonEmpty )
{  int count = 0;
   std::vector<ScalarRange> twoDim;
   twoDim.push_back( ScalarRange(0, 1, 0.125));
   twoDim.push_back( ScalarRange(0, -1, 0.125));
   MultiDimRange r( twoDim );

   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;

   ASSERT_EQ( 0,  count );

   r.reset();
   for ( ; ! r.isPastEnd(); r.nextValue() )
      ++count;

   ASSERT_EQ( 0,  count );
}

// test case 4: two dimensions: two non-empty ranges
TEST( MultiDimRangeMultiPass, TwoNonEmptyRanges )
{  int count = 0;
   std::vector<ScalarRange> twoDim;
   twoDim.push_back( ScalarRange(0, 1, 0.125));
   twoDim.push_back( ScalarRange(0, 2, 0.125));
   MultiDimRange r( twoDim );

   for ( ; ! r.isPastEnd(); r.nextValue() )
   {
      std::vector<double> values;
      r.getValue(values);

      ASSERT_EQ( 2, values.size() );
      ASSERT_EQ( static_cast<double>(count % 9) / 8,  values[0] );
      ASSERT_EQ( static_cast<double>(count / 9) / 8,  values[1] );

      ++count;
   }

   ASSERT_EQ( 9*17,  count );

   r.reset();
   count = 0;

   for ( ; ! r.isPastEnd(); r.nextValue() )
   {
      std::vector<double> values;
      r.getValue(values);

      ASSERT_EQ( 2, values.size() );
      ASSERT_EQ( static_cast<double>(count % 9) / 8,  values[0] );
      ASSERT_EQ( static_cast<double>(count / 9) / 8,  values[1] );

      ++count;
   }

   ASSERT_EQ( 9*17,  count );
}



