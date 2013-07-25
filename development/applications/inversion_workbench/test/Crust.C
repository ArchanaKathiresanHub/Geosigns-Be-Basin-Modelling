#include "Crust.h"
#include <iostream>
#include <cassert>
#include <iterator>

#include <gtest/gtest.h>

// Test input domain of Crust::overlap
//   0 == 1 == 2 == 3 == 4 == 5
//   |-A--| 
//             |-B--|
TEST( CrustOverlap, DisjointRight)
{
   EXPECT_FALSE( Crust::overlap(1, 1, 3, 1) );
}

//   |-A--| 
//        |-B--|
TEST( CrustOverlap, AdjoiningRight)
{
   EXPECT_FALSE( Crust::overlap(1, 1, 2, 1) );
}

//   |-A--| 
//     |-B--|
TEST( CrustOverlap, HalfOverlappingRight)
{
   EXPECT_TRUE( Crust::overlap(1, 1, 1.5, 1 ) );
}

//   |-A--| 
//   |-B--|
TEST( CrustOverlap, TheSame) 
{
   EXPECT_TRUE( Crust::overlap(1, 1, 1, 1 ) );
}

//   |---A----| 
//     |-B--|
TEST( CrustOverlap, Contains)
{
   EXPECT_TRUE( Crust::overlap(2, 2, 1.5, 1) );
}

//             |-A--|
//   |-B--| 
TEST( CrustOverlap, DisjointLeft)
{
   EXPECT_FALSE( Crust::overlap(4, 1, 2, 1) );
}

//        |-A--|
//   |-B--| 
TEST( CrustOverlap, AdjoiningLeft)
{
   EXPECT_FALSE( Crust::overlap(2, 1, 1, 1) );
}

//     |-A--|
//   |-B--| 
TEST( CrustOverlap, HalfOverlappingLeft )
{
   EXPECT_TRUE( Crust::overlap(2, 1, 1.5, 1 ) );
}

//        |-A--|
//      |---B----| 
TEST( CrustOverlap, FitsIn )
{
   EXPECT_TRUE( Crust::overlap(2, 1, 2.5, 2) );
}

// Error conditions: durations can't be negative
TEST( CrustOverlap, NegativeDuration )
{
   EXPECT_DEATH( Crust::overlap(2, -1, 1, 1), "");
   EXPECT_DEATH( Crust::overlap(2, 1, 1, -1), "");
}

#define CHECK(crust, series) \
   do { \
      std::vector< std::pair< double, double > > v = (crust).getThicknessHistory();\
      const size_t expectedSize = sizeof((series))/sizeof(*(series));\
      const size_t minSize = std::min( expectedSize, v.size() );\
      EXPECT_EQ( expectedSize , v.size() ); \
      EXPECT_TRUE( std::equal( v.begin(), v.begin() + minSize, (series)) ); \
   } while(0) 

TEST( Crust, addThinningEvent )
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
   EXPECT_ANY_THROW( crust.addThinningEvent( 65, 10, 2.0) );
   CHECK( crust, finalSeries );

   // test case 6: overlap with second event
   EXPECT_ANY_THROW( crust.addThinningEvent( 51.5, 1.0, 2.0 ) );
   CHECK(crust, finalSeries);

   // test case 7: overlap with third event
   EXPECT_ANY_THROW(  crust.addThinningEvent(50, 10, 5.0) );
   CHECK(crust, finalSeries);

   // test case 8: overlap with all events
   EXPECT_ANY_THROW( crust.addThinningEvent(200, 200, 5.0) );
   CHECK(crust, finalSeries);
}


TEST( Crust, getThicknessHistory )
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


