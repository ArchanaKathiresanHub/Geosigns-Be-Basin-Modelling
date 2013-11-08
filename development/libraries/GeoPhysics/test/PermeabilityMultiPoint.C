#include "../src/PermeabilityMultiPoint.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>

#include <gtest/gtest.h>

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


TEST( PermeabilityMultiPoint, permeability )
{
   //Input domain:
   // porosity and permeabitily samples - see above.
   // 0 <= calculatedPorosity < 1
   
   EXPECT_DOUBLE_EQ(3.51119173421513053E-01, PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.0 ));
   
   EXPECT_DOUBLE_EQ(1.99526231496887951, PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.05 ));
   
   
   EXPECT_DOUBLE_EQ(1.13382350121784938E+01, PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.1 ));
   
   EXPECT_DOUBLE_EQ(1000, PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 0.9999 ));

   EXPECT_DOUBLE_EQ(1000, PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeability( 0, 0, 1.0 ));

}

TEST( PermeabilityMultiPoint, permeabilityDerivative)
{
   //Input domain:
   // 0 <= calculatedPorosity < 1

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0, 1, permeability, derivative);
      EXPECT_DOUBLE_EQ(3.51119173421513053E-01, permeability);
      EXPECT_DOUBLE_EQ(1.22007249619155775E+01, derivative);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0.05, 1, permeability, derivative);
      EXPECT_DOUBLE_EQ(1.99526231496887951E+00, permeability);
      EXPECT_DOUBLE_EQ(6.93315790607256304E+01, derivative);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0.1, 1, permeability, derivative);
      EXPECT_DOUBLE_EQ(1.13382350121784938E+01, permeability);
      EXPECT_DOUBLE_EQ(3.93982150245844593E+02, derivative);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 0.9999, 1, permeability, derivative);
      EXPECT_DOUBLE_EQ(1000.0, permeability);
      EXPECT_DOUBLE_EQ(3.47481023124556013E+04, derivative);
   }

   {
      double permeability = NAN, derivative = NAN;
      PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).permeabilityDerivative( 0, 0, 1.0, 1, permeability, derivative);
      EXPECT_DOUBLE_EQ(1000.0, permeability);
      EXPECT_DOUBLE_EQ(3.47481023124556013E+04, derivative);
   }
}

TEST( PermeabilityMultiPoint, depoPerm )
{
   using namespace DataAccess::Interface;

   EXPECT_DOUBLE_EQ(8.59999999999999787E+00, PermeabilityMultiPoint( surfacePorosity, porositySamples, permeabilitySamples ).depoPerm());
}

TEST( PermeabilityMultiPoint, model)
{
   using namespace DataAccess::Interface;

   EXPECT_EQ( MULTIPOINT_PERMEABILITY, PermeabilityMultiPoint( 0.1, porositySamples, permeabilitySamples ).model());
}


