#include "ScalarRange.h"
#include <iostream>

#include <gtest/gtest.h>

// Note:
// The implementation of the class ScalarRange is so simple, that only
// testing the functions as a unit has no value. Here two cases are tested
// single-pass and multi-pass traversal of the range.

// test case 1a: empty
TEST( ScalarRangeSinglePass, emptyPositiveStep )
{  int count = 0;
  for (ScalarRange r(0,-1,1); !r.isPastEnd(); r.nextValue())
     count++;

  EXPECT_EQ(0, count );
}

// test case 1b: empty
TEST( ScalarRangeSinglePass, emptyNegativeStep )
{  int count = 0;
  for (ScalarRange r(0,1,-1); !r.isPastEnd(); r.nextValue())
     count++;

  EXPECT_EQ(0, count );
}

// test case 2: one-element, step = 0. This should crash the program
TEST( ScalarRangeSinglePass, StepZero)
{ 
   EXPECT_DEATH( ScalarRange r(0,0,0), "");
}

// test case 3a: one-element, step > 0
TEST( ScalarRangeSinglePass, OneElementPositiveStep)
{
   int count = 0;
   for (ScalarRange r( 0, 0, 0.125); !r.isPastEnd(); r.nextValue())
   {
      EXPECT_EQ( static_cast<double>(count) / 8 , r.getValue() );
      count++;
   }
   EXPECT_EQ(1, count );
}

// test case 3b: one-element, step < 0
TEST( ScalarRangeSinglePass, OneElementNegativeStep)
{
   int count = 0;
   for (ScalarRange r( 0, 0, -0.125); !r.isPastEnd(); r.nextValue())
   {
      EXPECT_EQ( static_cast<double>(count) / 8 , r.getValue() );
      count++;
   }
   EXPECT_EQ(1, count );
}

// test case 4a: multiple elements, inclusive at both ends
TEST( ScalarRangeSinglePass, MultipleElementsPositiveStep)
{
   int count = 0;
   for (ScalarRange r( 0, 1, 0.125); !r.isPastEnd(); r.nextValue())
   {
      EXPECT_EQ( static_cast<double>(count) / 8 , r.getValue() );
      count++;
   }
   EXPECT_EQ(9, count );
}

// test case 4b: multiple elements, inclusive at both ends
TEST( ScalarRangeSinglePass, MultipleElementsNegativeStep)
{
   int count = 9;
   for (ScalarRange r( 1, 0, -0.125); !r.isPastEnd(); r.nextValue())
   {
      count--;
      EXPECT_EQ( static_cast<double>(count) / 8 , r.getValue() );
   }
   EXPECT_EQ(0, count );
}

// test case 5: multiple elements, exclusive at end.
TEST( ScalarRangeSinglePass, MultipleElementsExclusiveEndPositiveStep)
{
   int count = 0;
   for (ScalarRange r( 0, 0.875 , 0.25); !r.isPastEnd(); r.nextValue())
   {
      EXPECT_EQ( static_cast<double>(count) / 4 , r.getValue() );
      count++;
   }
   EXPECT_EQ(4, count );
}

// test case 5: multiple elements, exclusive at end.
TEST( ScalarRangeSinglePass, MultipleElementsExclusiveEndNegativeStep)
{
   int count = 4;
   for (ScalarRange r( 1 , 0.125, -0.25); !r.isPastEnd(); r.nextValue())
   {
      EXPECT_EQ( static_cast<double>(count) / 4 , r.getValue() );
      count--;
   }
   EXPECT_EQ(0, count );
}

// test case 1a: empty
TEST( ScalarRangeMultiPass, EmptyNegativeStep )
{  int count = 0;
  ScalarRange r(0,-1, 1);
  for ( ; !r.isPastEnd(); r.nextValue())
     count++;

  r.reset();

  for ( ; !r.isPastEnd(); r.nextValue())
     count++;

  ASSERT_EQ( 0,  count );
}

// test case 1b: empty
TEST( ScalarRangeMultiPass, EmptyPositiveStep )
{  int count = 0;
  ScalarRange r(0,1, -1);
  for ( ; !r.isPastEnd(); r.nextValue())
     count++;

  r.reset();

  for ( ; !r.isPastEnd(); r.nextValue())
     count++;

  ASSERT_EQ( 0,  count );
}

// test case 2: one-element, step = 0;
TEST( ScalarRangeMultiPass, ZeroStep)
{ 
  ASSERT_DEATH( ScalarRange r(0,0,0), "" );
}

// test case 3a: one-element, step > 0
TEST( ScalarRangeMultiPass, OneElementPositiveStep)
{
   int count = 0;
   ScalarRange r( 0, 0, 0.125);
   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 1 ,  count );

   r.reset();

   count = 0;
   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 1 ,  count );
}

// test case 3b: one-element, step < 0
TEST( ScalarRangeMultiPass, OneElementNegativeStep)
{
   int count = 0;
   ScalarRange r( 0, 0, -0.125);
   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 1 ,  count );

   r.reset();

   count = 0;
   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 1 ,  count );
}

// test case 4a: multiple elements, inclusive at both ends
TEST( ScalarRangeMultiPass, MultipleElementsPositiveStep)
{
   int count = 0;
   ScalarRange r( 0, 1, 0.125);
      
   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 9 ,  count );

   r.reset();
   count = 0;

   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 9 ,  count );
}

// test case 4b: multiple elements, inclusive at both ends
TEST( ScalarRangeMultiPass, MultipleElementsNegativeStep)
{
   int count = 9;
   ScalarRange r( 1, 0, -0.125);
      
   for (; !r.isPastEnd(); r.nextValue())
   {
      count--;
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
   }
   ASSERT_EQ( 0 ,  count );

   r.reset();
   count = 9;

   for (; !r.isPastEnd(); r.nextValue())
   {
      count--;
      ASSERT_EQ( static_cast<double>(count) / 8 ,  r.getValue() );
   }
   ASSERT_EQ( 0 ,  count );
}

// test case 5a: multiple elements, exclusive at end.
TEST( ScalarRangeMultiPass, MultipleElementsExclusiveEndPositiveStep)
{
   int count = 0;
   ScalarRange r( 0, 0.875 , 0.25);
   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 4 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 4 ,  count );

   r.reset();
   count = 0;

   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 4 ,  r.getValue() );
      count++;
   }
   ASSERT_EQ( 4 ,  count );
}

// test case 5b: multiple elements, exclusive at end.
TEST( ScalarRangeMultiPass, MultipleElementsExclusiveEndNegativeStep)
{
   int count = 4;
   ScalarRange r( 1, 0.125 , -0.25);
   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 4 ,  r.getValue() );
      count--;
   }
   ASSERT_EQ( 0 ,  count );

   r.reset();
   count = 4;

   for (; !r.isPastEnd(); r.nextValue())
   {
      ASSERT_EQ( static_cast<double>(count) / 4 ,  r.getValue() );
      count--;
   }
   ASSERT_EQ( 0 ,  count );
}



