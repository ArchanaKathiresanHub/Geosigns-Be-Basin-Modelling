#include "Crust.h"
#include <iostream>
#include <cassert>

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


void test_Crust_addThinningEvent()
{
   bool r;

   Crust crust(100.0);

   assert( crust.m_thinningEvents.empty() );

   // test case 1: just one 
   r = crust.addThinningEvent(50, 10, 1);
   assert( r );
   assert( crust.m_thinningEvents.size() == 1);

   
   // test case 2: two disjoint events
   r = crust.addThinningEvent(70, 10, 1);
   assert( r );
   assert( crust.m_thinningEvents.size() == 2);

   // test case 3: one event colliding with one 
   r = crust.addThinningEvent(55, 10, 1);
   assert( !r );
   assert( crust.m_thinningEvents.size() == 2);

   



}


void test_Crust_getThicknessHistory()
{

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
