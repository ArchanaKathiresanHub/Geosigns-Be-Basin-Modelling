#include "ScalarRange.h"
#include <cassert>
#include <iostream>

// Note:
// The implementation of the class ScalarRange is so simple, that only
// testing the functions as a unit has no value. Here two cases are tested
// single-pass and multi-pass traversal of the range.

void test_ScalarRange_singlePass()
{
   // test case 1: empty
   {  int count = 0;
     for (ScalarRange r(0,-1,0); !r.isPastEnd(); r.nextValue())
        count++;

     assert( count == 0);
   }

   // test case 2: one-element, step = 0, so infinite iterations
   { 
     ScalarRange r(0,0,0);

     for (int count = 0; count < 1000;  ++count, r.nextValue() )
        assert( !r.isPastEnd() );
   }

   // test case 3: one-element, step > 0
   {
      int count = 0;
      for (ScalarRange r( 0, 0, 0.125); !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 1 );
   }

   // test case 4: multiple elements, inclusive at both ends
   {
      int count = 0;
      for (ScalarRange r( 0, 1, 0.125); !r.isPastEnd(); r.nextValue())
      {
         assert( r.getValue() == static_cast<double>(count) / 8 );
         count++;
      }
      assert( count == 9 );
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
}

void test_ScalarRange_multiPass()
{
   // test case 1: empty
   {  int count = 0;
     ScalarRange r(0,-1,0);
     for ( ; !r.isPastEnd(); r.nextValue())
        count++;

     r.reset();

     for ( ; !r.isPastEnd(); r.nextValue())
        count++;

     assert( count == 0);
   }

   // test case 2: one-element, step = 0, so infinite iterations
   { 
     ScalarRange r(0,0,0);

     for (int count = 0; count < 1000;  ++count, r.nextValue() )
        assert( !r.isPastEnd() );

     r.reset();

     for (int count = 0; count < 1000;  ++count, r.nextValue() )
        assert( !r.isPastEnd() );
   }

   // test case 3: one-element, step > 0
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

   // test case 4: multiple elements, inclusive at both ends
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

   // test case 5: multiple elements, exclusive at end.
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
