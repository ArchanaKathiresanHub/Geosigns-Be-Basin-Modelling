#include "../src/PermeabilitySandStone.h"
#include "UnitTestUtils.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>

using namespace GeoPhysics;

void test_PermeabilitySandStone_permeability()
{
   //Input domain:
   // permeabilityIncr = 1.5
   // depositionalPermeability = 6000 mD  
   // depositionalPorosity = 39%, 41%, 42%, 48%, 50%, 60%, 70%
   
   // ves: invalid (-infinity, -1.0e+5) [1.0e+5, 0). valid = {0}, (0, 1.0e+5), [1.0e+5, infinity)
   // maxVes: invalid = (-infinity, 0), {0}, [0, ves).  valid = [ves, infinity)
   // calculatedPorosity: invalid = (-infinity, 0), (1, infinity). valid = {0}, (0, 1), {1}
   
   double epsilon = std::numeric_limits<double>::epsilon();

   // The weird/invalid cases
   // varying ves invalid classes
   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.39, 6000, 1.5).permeability( -1e+6, 1.0e+5, 0)
         , 8.47522526773653202270e-03 , 16*epsilon
     );

   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.41, 6000, 1.5).permeability( -1e+5, 1.0e+5, 0.1)
         , 1.34323268314100546794e-01, 4*epsilon
     );

   // varying maxVes invalid classes
   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.48, 6000, 1.5).permeability( 0, -1e+6, 1)
         , 1000.0 , epsilon
     );

   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.50, 6000, 1.5).permeability( 1e+4, 0, 0)
         , 1.89736659610102758244e-04, 4*epsilon
     );

   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.60, 6000, 1.5).permeability( 1e+6, 1e+3, 0.3)
         , 1.89736659610102775808e-01, epsilon
     );

   // varying calculatedPorosity invalid classes
   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.70, 6000, 1.5).permeability( 0,0, -5)
         , 1.89736659610102764348e-82, 64*epsilon
     );

   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.35, 6000, 1.5).permeability( 1e+4, 1e+5, 5)
         , 1000.0, epsilon
     );

   // The normal cases
   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.39, 6000, 1.5).permeability( 1.0e+5, 1.0e+5, 0.0)
         , 8.47522526773653202270e-03 , 16*epsilon 
     );

   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.48, 6000, 1.5).permeability( 1.0e+6, 1.0e+6, 0.2)
        , 3.78574406688116593678e-01, 4*epsilon 
     );

   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.60, 6000, 1.5).permeability( 1.0e+6, 2.0e+6, 0.3) 
        , 1.89736659610102775808e-01, epsilon
     );

   ASSERT_ALMOST_EQUAL( PermeabilitySandStone( 0.60, 6000, 1.5).permeability( 1.0e+6, 2.0e+6, 0.6) 
         , 1000.0, epsilon
   );
}

void test_PermeabilitySandStone_permeabilityDerivative()
{
   //Input domain:
   // permeabilityIncr = 1.5
   // depositionalPermeability = 6000 mD  
   // depositionalPorosity = 39%, 41%, 42%, 48%, 50%, 60%, 70%
   
   // ves: invalid (-infinity, -1.0e+5) [1.0e+5, 0). valid = {0}, (0, 1.0e+5), [1.0e+5, infinity)
   // maxVes: invalid = (-infinity, 0), {0}, [0, ves).  valid = [ves, infinity)
   // calculatedPorosity: invalid = (-infinity, 0), (1, infinity). valid = {0}, (0, 1), {1}

   double epsilon = std::numeric_limits<double>::epsilon();
   
   // The weird cases
   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.39, 6000, 1.5).permeabilityDerivative( -1e+6, 1.0e+5, 0, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability, 8.47522526773653202270e-03, 16*epsilon);
      ASSERT_ALMOST_EQUAL( derivative, 2.92723910418849148432e-03, 16*epsilon);
   }

   {
      double permeability = NAN, derivative = NAN;

      PermeabilitySandStone( 0.41, 6000, 1.5).permeabilityDerivative( -1e+5, 1.0e+5, 0.1, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability, 1.34323268314100546794e-01, 8*epsilon);
      ASSERT_ALMOST_EQUAL( derivative, 4.63936132893431052437e-02, 8*epsilon);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.48, 6000, 1.5).permeabilityDerivative( 0, -1e+6, 1, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 1000.0 , epsilon);
      ASSERT_ALMOST_EQUAL( derivative, 1.30754967814368118286e+11, 4*epsilon);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.50, 6000, 1.5).permeabilityDerivative( 1e+4, 0, 0, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 1.89736659610102758244e-04, 4*epsilon);
      ASSERT_ALMOST_EQUAL( derivative, 6.55327206019062116467e-05, 4*epsilon);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1e+6, 1e+3, 0.3, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 1.89736659610102775808e-01, epsilon);
      ASSERT_ALMOST_EQUAL( derivative, 6.55327206019062219466e-02, epsilon);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.50, 6000, 1.5).permeabilityDerivative( 0, 0, -5, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 1.89736659610102742578e-79, 64*epsilon);
      ASSERT_ALMOST_EQUAL( derivative, 6.55327206019062006483e-80, 64*epsilon);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1.0e+5, 1.0e+5, 0.0, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 6.00000000000000015201e-06, 4*epsilon);
      ASSERT_ALMOST_EQUAL( derivative, 2.07232658369464115328e-06, 4*epsilon);
   }

   // The normal cases

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.39, 6000, 1.5).permeabilityDerivative( 1.0e+5, 1.0e+5, 0, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 8.47522526773653202270e-03, 16*epsilon );
      ASSERT_ALMOST_EQUAL( derivative , 2.92723910418849148432e-03 , 16*epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.48, 6000, 1.5).permeabilityDerivative( 1.0e+6, 1.0e+6, 0.2, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 3.78574406688116593678e-01, 4*epsilon );
      ASSERT_ALMOST_EQUAL( derivative , 1.30754967814368405765e-01 , 4*epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1.0e+6, 2.0e+6, 0.3, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 1.89736659610102775808e-01, epsilon );
      ASSERT_ALMOST_EQUAL( derivative , 6.55327206019062219466e-02 , epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1.0e+6, 2.0e+6, 0.6, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability , 1000.0, epsilon );
      ASSERT_ALMOST_EQUAL( derivative , 2.07232658369464115822e+03 , epsilon );
   }
}

void test_PermeabilitySandStone_depoPerm()
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   assert(PermeabilitySandStone( 0.60, 6000, 1.5).depoPerm() == 6000);
}

void test_PermeabilitySandStone_model()
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   assert(PermeabilitySandStone( 0.50, 6000, 1.5).model() == SANDSTONE_PERMEABILITY);
}

int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "permeability")==0)
      test_PermeabilitySandStone_permeability();
   else if (std::strcmp(argv[1], "permeabilityDerivative")==0)
      test_PermeabilitySandStone_permeabilityDerivative();
   else if (std::strcmp(argv[1], "depoPerm")==0)
      test_PermeabilitySandStone_depoPerm();
   else if (std::strcmp(argv[1], "model")==0)
      test_PermeabilitySandStone_model();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }



   return 0;
}
