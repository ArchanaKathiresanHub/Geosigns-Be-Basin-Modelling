#include "../src/PermeabilityImpermeable.h"

#include <iostream>
#include <cstring>
#include <cassert>
#include <cmath>

using namespace GeoPhysics;

void test_PermeabilityImpermeable_permeability()
{
   //Input domain:
   // ves >= 1.0e+5
   // maxVes >= ves
   // 0 <= calculatedPorosity < 1

   PermeabilityImpermeable p(0.0, DataAccess::Interface::IMPERMEABLE_PERMEABILITY);

   assert( p.permeability( 1.0e+5, 1.0e+5, 0) == 1.0e-9);
   assert( p.permeability( 1.0e+6, 1.0e+6, 0.2) == 1.0e-9);
   assert( p.permeability( 1.0e+6, 2.0e+6, 0.3) == 1.0e-9);
}

void test_PermeabilityImpermeable_permeabilityDerivative()
{
   //Input domain:
   // ves >= 1.0e+5
   // maxVes >= ves
   // 0 <= calculatedPorosity < 1

   PermeabilityImpermeable p(0.0, DataAccess::Interface::IMPERMEABLE_PERMEABILITY);

   {
      double permeability = NAN, derivative = NAN;
      p.permeabilityDerivative( 1.0e+5, 1.0e+5, 0, permeability, derivative);
      assert( permeability == 1e-9);
      assert( derivative == 0.0);
   }

   {
      double permeability = NAN, derivative = NAN;
      p.permeabilityDerivative( 1.0e+6, 1.0e+6, 0.2, permeability, derivative);
      assert( permeability == 1e-9);
      assert( derivative == 0.0);
   }

   {
      double permeability = NAN, derivative = NAN;
      p.permeabilityDerivative( 1.0e+6, 2.0e+6, 0.3, permeability, derivative);
      assert( permeability == 1e-9);
      assert( derivative == 0.0);
   }
}

void test_PermeabilityImpermeable_depoPerm()
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   assert( PermeabilityImpermeable(0.0, IMPERMEABLE_PERMEABILITY).depoPerm() == 0.0);
   assert( PermeabilityImpermeable(5.0, NONE_PERMEABILITY).depoPerm() == 5.0);
}

void test_PermeabilityImpermeable_model()
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   assert( PermeabilityImpermeable(0.0, IMPERMEABLE_PERMEABILITY).model() == IMPERMEABLE_PERMEABILITY );
   assert( PermeabilityImpermeable(5.0, NONE_PERMEABILITY).model() == NONE_PERMEABILITY);
}

int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "permeability")==0)
      test_PermeabilityImpermeable_permeability();
   else if (std::strcmp(argv[1], "permeabilityDerivative")==0)
      test_PermeabilityImpermeable_permeabilityDerivative();
   else if (std::strcmp(argv[1], "depoPerm")==0)
      test_PermeabilityImpermeable_depoPerm();
   else if (std::strcmp(argv[1], "model")==0)
      test_PermeabilityImpermeable_model();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }



   return 0;
}
