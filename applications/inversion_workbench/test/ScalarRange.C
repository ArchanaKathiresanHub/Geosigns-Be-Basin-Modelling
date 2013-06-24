#include "ScalarRange.h"
#include <cassert>
#include <iostream>

// Note:
// The implementation of the class ScalarRange is so simple, that only
// testing the functions as a unit has no value. Here two cases are tested
// single-pass and multi-pass traversal of the range.

void test_ScalarRange_singlePass()
{
   // test case 1a: empty
   {  int count = 0;
     for (ScalarRange r(0,-1,1); !r.isPastEnd(); r.nextValue())
        count++;

     assert( count == 0);
   }

   // test case 1b: empty
   {  int count = 0;
     for (ScalarRange r(0,1,-1); !r.isPastEnd(); r.nextValue())
        count++;

     assert( count == 0);
   }

   // DISABLED test case 2: one-element, step = 0, so infinite iterations
   // FIXME: Because ScalarRange uses 'assert' to check whether 
   // step zero, this is difficult to test. Perhaps other Unit Test 
   // framework can handle this.
   //{ 
   //  ScalarRange r(0,0,0);
   //
   //  for (int count = 0; count < 1000;  ++count, r.nextValue() )
   //     assert( !r.isPastEnd() );
   //}

   // test case 3a: one-element, step > 0
   {
      int count = 0;
      for (ScalarRange r( 0, 0, 0.125); !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 1 );
   }

   // test case 3b: one-element, step < 0
   {
      int count = 0;
      for (ScalarRange r( 0, 0, -0.125); !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 1 );
   }

   // test case 4a: multiple elements, inclusive at both ends
   {
      int count = 0;
      for (ScalarRange r( 0, 1, 0.125); !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 9 );
   }

   // test case 4b: multiple elements, inclusive at both ends
   {
      int count = 9;
      for (ScalarRange r( 1, 0, -0.125); !r.isPastEnd(); r.nextValue())
      {
         count--;
         assert( r.getValue() == static_cast<double>(count) / 8 );
      }
      assert( count == 0 );
   }

   // test case 5: multiple elements, exclusive at end.
   {
      int count = 0;
      for (ScalarRange r( 0, 0.875 , 0.25); !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 4 );
         count++;
      }
      assert( count == 4 );
   }

   // test case 5: multiple elements, exclusive at end.
   {
      int count = 4;
      for (ScalarRange r( 1 , 0.125, -0.25); !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 4 );
         count--;
      }
      assert( count == 0);
   }
}

void test_ScalarRange_multiPass()
{
   // test case 1a: empty
   {  int count = 0;
     ScalarRange r(0,-1, 1);
     for ( ; !r.isPastEnd(); r.nextValue())
        count++;

     r.reset();

     for ( ; !r.isPastEnd(); r.nextValue())
        count++;

     assert( count == 0);
   }

   // test case 1b: empty
   {  int count = 0;
     ScalarRange r(0,1, -1);
     for ( ; !r.isPastEnd(); r.nextValue())
        count++;

     r.reset();

     for ( ; !r.isPastEnd(); r.nextValue())
        count++;

     assert( count == 0);
   }

   // FIXME: Have unit test framework that can handle failing assertions 
   // somewhere else in the code.
   // DISABLED: test case 2: one-element, step = 0, so infinite iterations
   //{ 
   //  ScalarRange r(0,0,0);
   //
   //  for (int count = 0; count < 1000;  ++count, r.nextValue() )
   //     assert( !r.isPastEnd() );
   //
   //  r.reset();
   //
   //  for (int count = 0; count < 1000;  ++count, r.nextValue() )
   //     assert( !r.isPastEnd() );
   // }

   // test case 3a: one-element, step > 0
   {
      int count = 0;
      ScalarRange r( 0, 0, 0.125);
      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 1 );

      r.reset();

      count = 0;
      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 1 );
   }

   // test case 3b: one-element, step < 0
   {
      int count = 0;
      ScalarRange r( 0, 0, -0.125);
      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 1 );

      r.reset();

      count = 0;
      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 1 );
   }

   // test case 4a: multiple elements, inclusive at both ends
   {
      int count = 0;
      ScalarRange r( 0, 1, 0.125);
         
      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 9 );

      r.reset();
      count = 0;

      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 9 );
   }

   // test case 4b: multiple elements, inclusive at both ends
   {
      int count = 9;
      ScalarRange r( 1, 0, -0.125);
         
      for (; !r.isPastEnd(); r.nextValue())
      {
         count--;
         assert( r.getValue() == static_cast<double>(count) / 8 );
      }
      assert( count == 0 );

      r.reset();
      count = 9;

      for (; !r.isPastEnd(); r.nextValue())
      {
         count--;
         assert( r.getValue() == static_cast<double>(count) / 8 );
      }
      assert( count == 0 );
   }

   // test case 5a: multiple elements, exclusive at end.
   {
      int count = 0;
      ScalarRange r( 0, 0.875 , 0.25);
      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 4 );
         count++;
      }
      assert( count == 4 );

      r.reset();
      count = 0;

      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 4 );
         count++;
      }
      assert( count == 4 );
   }

   // test case 5b: multiple elements, exclusive at end.
   {
      int count = 4;
      ScalarRange r( 1, 0.125 , -0.25);
      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 4 );
         count--;
      }
      assert( count == 0 );

      r.reset();
      count = 4;

      for (; !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 4 );
         count--;
      }
      assert( count == 0 );
   }
}



int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "singlePass")==0)
      test_ScalarRange_singlePass();
   else if (std::strcmp(argv[1], "multiPass")==0)
      test_ScalarRange_multiPass();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }

   return 0;
}
