#include "MultiDimRange.h"
#include <cassert>
#include <iostream>

// Note:
// The implementation of the class MultiDimRange is so simple, that only
// testing the functions as a unit has no value. Here two cases are tested
// single-pass and multi-pass traversal of the range.

void test_MultiDimRange_singlePass()
{
   // test case 1: null-set

   {  int count = 0;
      std::vector<ScalarRange> empty;
      MultiDimRange r( empty  );

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;

      assert( count == 0);
   }

   // test case 2: one dimension: one empty range
   {  int count = 0;
      std::vector<ScalarRange> oneDimension(1, ScalarRange(0, -1, 0));
      MultiDimRange r( oneDimension );

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;

      assert( count == 0);
   }

   // test case 2: one dimension: one non-empty range
   {  int count = 0;
      std::vector<ScalarRange> oneDimension(1, ScalarRange(0, 1, 0.125));
      MultiDimRange r( oneDimension );

      for ( ; ! r.isPastEnd(); r.nextValue() )
      {
         std::vector<double> values;
         r.getValue(values);
         assert( values.size() == 1);
         assert( values[0] == static_cast<double>(count) / 8 );
         ++count;
      }

      assert( count == 9);
   }


   // test case 3: two dimensions: one empty range, one non-empty
   {  int count = 0;
      std::vector<ScalarRange> twoDim;
      twoDim.push_back( ScalarRange(0, 1, 0.125));
      twoDim.push_back( ScalarRange(0, -1, 0.125));
      MultiDimRange r( twoDim );

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;

      assert( count == 0);
   }

   // test case 4: two dimensions: two non-empty ranges
   {  int count = 0;
      std::vector<ScalarRange> twoDim;
      twoDim.push_back( ScalarRange(0, 1, 0.125));
      twoDim.push_back( ScalarRange(0, 2, 0.125));
      MultiDimRange r( twoDim );

      for ( ; ! r.isPastEnd(); r.nextValue() )
      {
         std::vector<double> values;
         r.getValue(values);

         assert(values.size() == 2);
         assert( values[0] == static_cast<double>(count % 9) / 8);
         assert( values[1] == static_cast<double>(count / 9) / 8);

         ++count;
      }

      assert( count == 9*17);
   }

}

void test_MultiDimRange_multiPass()
{
   // test case 1: null-set

   {  int count = 0;
      std::vector<ScalarRange> empty;
      MultiDimRange r( empty  );

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;
      assert( count == 0);

      r.reset();

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;

      assert( count == 0);
   }

   // test case 2: one dimension: one empty range
   {  int count = 0;
      std::vector<ScalarRange> oneDimension(1, ScalarRange(0, -1, 0));
      MultiDimRange r( oneDimension );

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;
      assert( count == 0);

      r.reset();

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;

      assert( count == 0);
   }

   // test case 2: one dimension: one non-empty range
   {  int count = 0;
      std::vector<ScalarRange> oneDimension(1, ScalarRange(0, 1, 0.125));
      MultiDimRange r( oneDimension );

      for ( ; ! r.isPastEnd(); r.nextValue() )
      {
         std::vector<double> values;
         r.getValue(values);
         assert( values.size() == 1);
         assert( values[0] == static_cast<double>(count) / 8 );
         ++count;
      }

      assert( count == 9);

      r.reset();
      count = 0;

      for ( ; ! r.isPastEnd(); r.nextValue() )
      {
         std::vector<double> values;
         r.getValue(values);
         assert( values.size() == 1);
         assert( values[0] == static_cast<double>(count) / 8 );
         ++count;
      }
   }


   // test case 3: two dimensions: one empty range, one non-empty
   {  int count = 0;
      std::vector<ScalarRange> twoDim;
      twoDim.push_back( ScalarRange(0, 1, 0.125));
      twoDim.push_back( ScalarRange(0, -1, 0.125));
      MultiDimRange r( twoDim );

      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;

      assert( count == 0);

      r.reset();
      for ( ; ! r.isPastEnd(); r.nextValue() )
         ++count;

      assert( count == 0);
   }

   // test case 4: two dimensions: two non-empty ranges
   {  int count = 0;
      std::vector<ScalarRange> twoDim;
      twoDim.push_back( ScalarRange(0, 1, 0.125));
      twoDim.push_back( ScalarRange(0, 2, 0.125));
      MultiDimRange r( twoDim );

      for ( ; ! r.isPastEnd(); r.nextValue() )
      {
         std::vector<double> values;
         r.getValue(values);

         assert(values.size() == 2);
         assert( values[0] == static_cast<double>(count % 9) / 8);
         assert( values[1] == static_cast<double>(count / 9) / 8);

         ++count;
      }

      assert( count == 9*17);

      r.reset();
      count = 0;

      for ( ; ! r.isPastEnd(); r.nextValue() )
      {
         std::vector<double> values;
         r.getValue(values);

         assert(values.size() == 2);
         assert( values[0] == static_cast<double>(count % 9) / 8);
         assert( values[1] == static_cast<double>(count / 9) / 8);

         ++count;
      }

      assert( count == 9*17);
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
      test_MultiDimRange_singlePass();
   else if (std::strcmp(argv[1], "multiPass")==0)
      test_MultiDimRange_multiPass();
   else 
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }

   return 0;
}
