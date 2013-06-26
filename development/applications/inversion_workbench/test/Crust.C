#include "Crust.h"
#include <iostream>
#include <cassert>
#include <iterator>

void test_Crust_overlap()
{
   // input domain coverage
   //   0 == 1 == 2 == 3 == 4 == 5
   //   |-A--| 
   //             |-B--|
   assert( ! Crust::overlap(1, 1, 3, 1) );

   //   |-A--| 
   //        |-B--|
   assert( ! Crust::overlap(1, 1, 2, 1) );

   //   |-A--| 
   //     |-B--|
   assert( Crust::overlap(1, 1, 1.5, 1 ) );

   //   |-A--| 
   //   |-B--|
   assert( Crust::overlap(1, 1, 1, 1 ) );

   //   |---A----| 
   //     |-B--|
   assert( Crust::overlap(2, 2, 1.5, 1) );

   //             |-A--|
   //   |-B--| 
   assert( ! Crust::overlap(4, 1, 2, 1) );

   //        |-A--|
   //   |-B--| 
   assert( ! Crust::overlap(2, 1, 1, 1) );

   //     |-A--|
   //   |-B--| 
   assert( Crust::overlap(2, 1, 1.5, 1 ) );

   //        |-A--|
   //      |---B----| 
   assert( Crust::overlap(2, 1, 2.5, 2) );
}

#define CHECK(crust, series) \
   do { \
      std::vector< std::pair< double, double > > v = (crust).getThicknessHistory();\
      assert( v.size() == (sizeof((series))/sizeof((series)[0]))); \
      assert( std::equal( v.begin(), v.end(), (series)) ); \
   } while(0) 

void test_Crust_addThinningEvent()
{
   typedef std::pair< double, double > TT;
   typedef std::vector<TT> V;

   Crust crust(100.0);

   {
      TT series[] = { TT(0, 100.0) };
      CHECK( crust, series );
   }

   // test case 1: just one 
   crust.addThinningEvent(50, 10, 2);
   {
      TT series[] = { TT(50, 100.0), TT(40, 50.0) };
      CHECK( crust, series );
   }

   // test case 2: two disjoint events
   crust.addThinningEvent(70, 10, 5);
   {
      TT series[] = { TT(70, 100), TT(60, 20.0), TT(50, 20.0), TT(40, 10.0) };
      CHECK( crust, series );
   }


   // test case 3: two disjoint events, one pre-adjoining event
   crust.addThinningEvent(51, 1, 2.0);
   {
      TT series[] = { TT(70, 100), TT(60, 20.0), TT(51, 20.0), TT(50, 10.0), TT(40, 5.0) };
      CHECK( crust, series );
   }

   // test case 4: two disjoint events, two adjoining event (before and after)
   crust.addThinningEvent(40, 1, 5.0);
   {
      TT series[] = { TT(70, 100), TT(60, 20.0), TT(51, 20.0), TT(50, 10.0), TT(40, 5.0), TT(39, 1.0) };
      CHECK( crust, series );
   }

   TT finalSeries[] = { TT(70, 100), TT(60, 20.0), TT(51, 20.0), TT(50, 10.0), TT(40, 5.0), TT(39, 1.0) };
   // test case 5: overlap with first event 
   bool caught = false;
   try
   {
      crust.addThinningEvent( 65, 10, 2.0);
   }
   catch(...)
   {
      caught = true;
   }
   CHECK( crust, finalSeries );
   assert(caught);

   // test case 6: overlap with second event
   caught = false;
   try
   {
      crust.addThinningEvent( 51.5, 1.0, 2.0 );
   }
   catch(...)
   {
      caught = true;
   }
   assert(caught);
   CHECK(crust, finalSeries);

   // test case 7: overlap with third event
   caught = false;
   try
   {
      crust.addThinningEvent(50, 10, 5.0);
   }
   catch(...)
   {
      caught = true;
   }
   assert(caught);
   CHECK(crust, finalSeries);

   // test case 8: overlap with all events
   caught = false;
   try
   {
      crust.addThinningEvent(200, 200, 5.0);
   }
   catch(...)
   {
      caught = true;
   }
   assert(caught);
   CHECK(crust, finalSeries);

}


void test_Crust_getThicknessHistory()
{
   typedef std::pair< double, double > TT;
   typedef std::vector<TT> V;

   Crust crust(100.0);

   {
      TT series[] = { TT(0, 100.0) };
      CHECK( crust, series );
   }

   // test case 1: just one 
   crust.addThinningEvent(50, 10, 2);
   {
      TT series[] = { TT(50, 100.0), TT(40, 50.0) };
      CHECK( crust, series );
   }

   // test case 2: two disjoint events
   crust.addThinningEvent(70, 10, 5);
   {
      TT series[] = { TT(70, 100), TT(60, 20.0), TT(50, 20.0), TT(40, 10.0) };
      CHECK( crust, series );
   }


   // test case 4: two disjoint events, one almost adjoining event
   crust.addThinningEvent(51, 1 - 1.1 * Crust::MinimumEventSeparation, 2.0);
   {
      TT series[] = { TT(70, 100), TT(60, 20.0), TT(51, 20.0), TT(50 + 1.1 * Crust::MinimumEventSeparation, 10.0)
         , TT(50, 10.0), TT(40, 5.0) };
      CHECK( crust, series );
   }

   // test case 5: two disjoint events, one adjoining event, and one almost adjoining event
   crust.addThinningEvent(52, 1.0 - .9 * Crust::MinimumEventSeparation, 5.0);
   {
      TT series[] = { TT(70, 100), TT(60, 20.0), TT(52, 20.0), TT(51+.9*Crust::MinimumEventSeparation, 4.0), 
         TT(50 + 1.1 * Crust::MinimumEventSeparation, 2.0) , TT(50, 2.0), TT(40, 1.0) };
      CHECK( crust, series );
   }
}



int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "overlap")==0)
      test_Crust_overlap();
   else if (std::strcmp(argv[1], "addThinningEvent") == 0)
      test_Crust_addThinningEvent();
   else if (std::strcmp(argv[1], "getThicknessHistory") == 0)
      test_Crust_getThicknessHistory();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }

   return 0;
}
