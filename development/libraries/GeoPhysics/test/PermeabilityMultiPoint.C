#include "../src/PermeabilityMultiPoint.h"
#include "UnitTestUtils.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>

using namespace GeoPhysics;

const double sm_sandstone_porositySamples[2] = { 0.05, 0.6 };    
const double sm_sandstone_permeabilitySamples[2] = { 3.0e-01, 8.60e+00 };
const double sm_sandstone_surfacePorosity = 0.6;

const double sm_mudst_40_porositySamples[2] = { 0.05, 0.6 };    
const double sm_mudst_40_permeabilitySamples[2] = { -6.0e+00, -0.5e+00 };
const double sm_mudst_40_surfacePorosity = 0.5632;

const double sm_mudst_50_porositySamples[2] = { 0.05, 0.6 };    
const double sm_mudst_50_permeabilitySamples[2] = { -6.1e+00 -1.7e+00 };
const double sm_mudst_50_surfacePorosity = 0.62;

const double sm_mudst_60_porositySamples[6] = { 0.05, 0.2, 0.3, 0.4, 0.5, 0.6 };
const double sm_mudst_60_permeabilitySamples[6] = { -5.93e+00, -5.23e+00, -4.67e+00, -3.9e+00, -3.0e+00, -2.0e+00 };
const double sm_mudst_60_surfacePorosity = 0.6685;

const std::vector<double> porositySamples( sm_sandstone_porositySamples, sm_sandstone_porositySamples + 2 );
const std::vector<double> permeabilitySamples( sm_sandstone_permeabilitySamples, sm_sandstone_permeabilitySamples + 2 );
const double surfacePorosity = sm_sandstone_surfacePorosity;


void test_PermeabilityMultiPoint_permeability()
{
   //Input domain:
   // porosity and permeabitily samples - see above.
   // 0 <= calculatedPorosity < 1
   
   double epsilon = std::numeric_limits<double>::epsilon();

   ASSERT_ALMOST_EQUAL( PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.0 ),
                        3.51119173421513053E-01, epsilon );
   
   ASSERT_ALMOST_EQUAL( PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.05 ),
                        1.99526231496887951, epsilon );
   
   
   ASSERT_ALMOST_EQUAL( PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.1 ),
                        1.13382350121784938E+01, epsilon );
   
   ASSERT_ALMOST_EQUAL( PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.9999 ),
                        1000, epsilon );

   ASSERT_ALMOST_EQUAL( PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 1.0 ),
                        1000, epsilon );

}

void test_PermeabilityMultiPoint_permeabilityDerivative()
{
   //Input domain:
   // 0 <= calculatedPorosity < 1

   double epsilon = std::numeric_limits<double>::epsilon();

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability, 3.51119173421513053E-01, epsilon );
      ASSERT_ALMOST_EQUAL( derivative, 1.22007249619155775E+01, epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0.05, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability, 1.99526231496887951E+00, epsilon );
      ASSERT_ALMOST_EQUAL( derivative, 6.93315790607256304E+01, epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0.1, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability, 1.13382350121784938E+01, epsilon );
      ASSERT_ALMOST_EQUAL( derivative, 3.93982150245844593E+02, epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0.9999, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability, 1000.0, epsilon );
      ASSERT_ALMOST_EQUAL( derivative, 3.47481023124556013E+04, epsilon );
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 1.0, permeability, derivative);
      ASSERT_ALMOST_EQUAL( permeability, 1000.0, epsilon );
      ASSERT_ALMOST_EQUAL( derivative, 3.47481023124556013E+04, epsilon );
   }
}

void test_PermeabilityMultiPoint_depoPerm()
{
   using namespace DataAccess::Interface;
   double epsilon = std::numeric_limits<double>::epsilon();

   ASSERT_ALMOST_EQUAL( PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).depoPerm(), 8.59999999999999787E+00, epsilon );
}

void test_PermeabilityMultiPoint_model()
{
   using namespace DataAccess::Interface;

   assert( PermeabilityMultiPoint( 0.1, porositySamples, permeabilitySamples ).model() == MULTIPOINT_PERMEABILITY);
}

int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "permeability")==0)
      test_PermeabilityMultiPoint_permeability();
   else if (std::strcmp(argv[1], "permeabilityDerivative")==0)
      test_PermeabilityMultiPoint_permeabilityDerivative();
   else if (std::strcmp(argv[1], "depoPerm")==0)
      test_PermeabilityMultiPoint_depoPerm();
   else if (std::strcmp(argv[1], "model")==0)
      test_PermeabilityMultiPoint_model();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }



   return 0;
}
