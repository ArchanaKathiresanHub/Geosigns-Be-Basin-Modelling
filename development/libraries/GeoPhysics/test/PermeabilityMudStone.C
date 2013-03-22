#include "../src/PermeabilityMudStone.h"
#include "UnitTestUtils.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>

using namespace GeoPhysics;

void test_PermeabilityMudStone_permeability()
{
   //Input domain:
   // permeabilityIncr = 1.5
   // permeabilityDecr = 0.01
   // depositionalPermeability = 0.01 mD (Shale, Marl, Dolo Mud, Lime Mud), 0.1 mD (chalk), 50 mD (siltstone)
   
   // ves >= 1.0e+5
   // maxVes >= ves
   // 0 <= calculatedPorosity < 1
   
   double epsilon = std::numeric_limits<double>::epsilon();

   ASSERT_ALMOST_EQUAL( PermeabilityMudStone( 0.01, 1.5, 0.01).permeability( 1.0e+5, 1.0e+5, 0.0)
         , 3.53553390593273810655e-03 , epsilon 
     );

   ASSERT_ALMOST_EQUAL( PermeabilityMudStone( 0.1, 1.5, 0.01).permeability( 1.0e+6, 1.0e+6, 0.2)
         , 2.74101222343421484229e-03 , epsilon 
     );


   ASSERT_ALMOST_EQUAL( PermeabilityMudStone( 50, 1.5, 0.01).permeability( 1.0e+6, 2.0e+6, 0.3)
         , 5.22936948520120403749e-01 , epsilon 
     );

   ASSERT_ALMOST_EQUAL( PermeabilityMudStone( 6000, 1.5, 0.01).permeability( 1.0e+5, 1.0e+5, 0.6) 
         , 1000.0, epsilon
   );
}

void test_PermeabilityMudStone_permeabilityDerivative()
{
   //Input domain:
   // ves >= 1.0e+5
   // maxVes >= ves
   // 0 <= calculatedPorosity < 1

   double epsilon = std::numeric_limits<double>::epsilon();
   
   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMudStone( 0.01, 1.5, 0.01).permeabilityDerivative( 1.0e+5, 1.0e+5, 0, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 3.53553390593273810655e-03, epsilon );
      ASSERT_ALMOST_EQUAL( derivative , -2.65165042944955341792e-08 , epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMudStone( 0.1, 1.5, 0.01).permeabilityDerivative( 1.0e+6, 1.0e+6, 0.2, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 2.74101222343421484229e-03, epsilon );
      ASSERT_ALMOST_EQUAL( derivative , -3.73774394104665699612e-09 , epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMudStone( 50, 1.5, 0.01).permeabilityDerivative( 1.0e+6, 2.0e+6, 0.3, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 5.22936948520120403749e-01, epsilon );
      ASSERT_ALMOST_EQUAL( derivative , -4.75397225927382145993e-09 , epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMudStone( 6000, 1.5, 0.01).permeabilityDerivative( 1.0e+5, 1.0e+5, 0.6, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 1000.0, epsilon );
      ASSERT_ALMOST_EQUAL( derivative , -1.59099025766973219131e-02 , epsilon );
   }
}

void test_PermeabilityMudStone_depoPerm()
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   assert(PermeabilityMudStone( 0.01, 1.5, 0.01).depoPerm() == 0.01);
}

void test_PermeabilityMudStone_model()
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   assert(PermeabilityMudStone( 0.01, 1.5, 0.01).model() == MUDSTONE_PERMEABILITY);
}

int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "permeability")==0)
      test_PermeabilityMudStone_permeability();
   else if (std::strcmp(argv[1], "permeabilityDerivative")==0)
      test_PermeabilityMudStone_permeabilityDerivative();
   else if (std::strcmp(argv[1], "depoPerm")==0)
      test_PermeabilityMudStone_depoPerm();
   else if (std::strcmp(argv[1], "model")==0)
      test_PermeabilityMudStone_model();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }



   return 0;
}
