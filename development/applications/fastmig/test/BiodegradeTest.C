/*!
* \file Biodegrade.C
* \brief This tests aim at evaluating the final result of biodegradation (lostComponents = amount of mass biodegradaded)
* \author Guillaume Vix guillaume.vix@pds.nl
* \date July 2015
*/

#include "../src/Biodegrade.h"
#include "Interface/BioConsts.h"

#include <gtest/gtest.h>

using namespace migration;
namespace DataAccess {
namespace Interface {

// Construction of BioConst for each component in the following order: Asphaltene, Resins, C15 + Aro, C15 + Sat, C6 - 14 Aro, C6 - 14 Sat, C5, C4, C3, C2, C1, COx, N2
const double myconstBio[] = { 0.1, 0.061, 0.05, 0.011, 0.007, 0.009, 0.001, 0.0008, 0.0003, 0.0005, 0.0008, 0.0, 0.001 };
std::vector<double> constBio(myconstBio, myconstBio + sizeof(myconstBio) / sizeof(double));
BioConsts bioConsts = BioConsts(70.0, constBio);
 

const unsigned int NumComponents = 23;
double inputComponents[NumComponents];
double lostComponents[NumComponents];
unsigned int component;


// This test checks the biodegradation computation with very simple/synthetic values
TEST(Biodegrade, biodegradation_computation_synthetic_data)
{
   // First test: all components have initialy the same mass
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents[component] = 0.0;
   }

   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(1.0, 55.0, inputComponents, lostComponents);

   double expectedResults[NumComponents] = { 0.0145529741736539, 0.348929710298962, 0.856661585310736, 23.081331256783, 34.0877224922136, 27.9284200135241, 72.0367988712538,
      74.8881728268666, 84.4660815127512, 80.0268313782813, 74.8881728268666, 100, 72.0367988712538, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

   for (component = 0; component < NumComponents; ++component)
   {
      EXPECT_NEAR(expectedResults[component], lostComponents[component], 1e-12);
   }

   // Second test: the mass vary without logic between components
   double inputComponents[NumComponents] = { 15000, 750, 5632, 2548, 10, 254, 369874, 0.1, 0.0, 175, 1000000, 153, 0.0000001,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
   double lostComponents[NumComponents] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

   myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(5.0, 79.9, inputComponents, lostComponents);

   double expectedResults2[NumComponents] = { 0.0, 7.5289774414955000E-10, 9.7227228934571000E-07, 3.6886161881690100E+01, 9.1836673484161200E-01, 9.3107058810331500E+00, 3.3829807026248200E+05, 9.4551510083537900E-02,
      0.0, 1.7152172752004700E+02, 9.4551510083537900E+05, 153.0, 9.1463057760881300E-08, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

   for (component = 0; component < NumComponents; ++component)
   {
      EXPECT_NEAR(expectedResults2[component], lostComponents[component], 1e-9);
   }
}


// This test checks the biodegradation computation with real data from a trap of the Marmul project
TEST(Biodegrade, biodegradation_computation_real_data)
{
   double inputComponents[NumComponents] = { 6.73855E-06, 0.000243952, 0.000112903, 0.002650752, 2.52951E-05, 4.07030506, 3.488207916, 87.12728357, 
      1305.47109, 87351.04848, 118713.8906, 0.0, 0.203121715, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
   double lostComponents[NumComponents] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(9.0, 73.405815, inputComponents, lostComponents);

   double expectedResults[NumComponents] = { 7.83929E-13, 2.58299E-08, 8.16667E-08, 0.001350157, 2.00268E-05, 2.64141794, 3.487285803, 
      87.11817417, 1305.469084, 87349.81165, 118701.4787, 0.0, 0.20306802, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

   EXPECT_NEAR(expectedResults[0], lostComponents[0], 1e-17);
   for (component = 1; component < 5; ++component)
   {
      EXPECT_NEAR(expectedResults[component], lostComponents[component], 1e-9);
   }
   for (component = 5; component < NumComponents; ++component)
   {
      EXPECT_NEAR(expectedResults[component], lostComponents[component], 1e-4);
   }
}


// This test checks the trend impact of each variable of the equation
// (the trap temperature, the temperature Factor (FT), the time Factor (Ft), the time Interval (Myr) and the Biodegradation coefficients (FBi))
TEST(Biodegrade, biodegradation_computation_variable_trend)
{
   double lostComponents1[NumComponents];
   double lostComponents2[NumComponents];
   
   // 1. Increasing the trap temperature should imply a decrease of the biodegradation
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents1[component] = 0.0;
      lostComponents2[component] = 0.0;
   }
   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(0.1, 35.0, inputComponents, lostComponents1); // with T1 = 35.0 °C
   myBiodegrade.calculate(0.1, 35.0001, inputComponents, lostComponents2); // with T2 > T1 (35.0001°C > 35.0°C)
  
   for (component = 0; component < 13; ++component) // only the 13 first components with BioConst are interesting here
   {
      if(component != 11)  // no need to test on CO2 (should be equal and tested in another test)
         ASSERT_GT(lostComponents1[component], lostComponents2[component]);
   }


   // 2. Increasing the temperature Factor (FT) should imply a decrease of the biodegradation
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents1[component] = 0.0;
      lostComponents2[component] = 0.0;
   }
   BioConsts bioConsts1 = BioConsts(70.0, constBio); // with Temp. Factor 1 = 70
   Biodegrade myBiodegrade1 = Biodegrade(80.0, bioConsts1, 0.5);
   myBiodegrade1.calculate(1, 55.0, inputComponents, lostComponents1); 
   
   BioConsts bioConsts2 = BioConsts(70.0001, constBio); // with Temp. Factor 2 = 70.0001 > Temp. Factor 1
   Biodegrade myBiodegrade2 = Biodegrade(80.0, bioConsts2, 0.5);
   myBiodegrade2.calculate(1, 55.0, inputComponents, lostComponents2);

   for (component = 0; component < 13; ++component) // only the 13 first components with BioConst are interesting here
   {
      if (component != 11) // no need to test on CO2 (should be equal and tested in another test)
         ASSERT_GT(lostComponents1[component], lostComponents2[component]);
   }


   // 3. Increasing the time Factor (Ft) should imply an increase of the biodegradation
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents1[component] = 0.0;
      lostComponents2[component] = 0.0;
   }
   myBiodegrade1 = Biodegrade(80.0, bioConsts, 0.5); // with time Factor 1 = 0.5
   myBiodegrade1.calculate(1, 55.0, inputComponents, lostComponents1);

   myBiodegrade2 = Biodegrade(80.0, bioConsts, 0.50001); // with time Factor 2 = 0.50001 > time Factor 1
   myBiodegrade2.calculate(1, 55.0, inputComponents, lostComponents2);

   for (component = 0; component < 13; ++component) // only the 13 first components with BioConst are interesting here
   {
      if (component != 11)  //no need to test on CO2 (should be equal and tested in another test)
         ASSERT_LT(lostComponents1[component], lostComponents2[component]);
   }


   // 4. Increasing the time Interval (Myr) should imply an increase of the biodegradation
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents1[component] = 0.0;
      lostComponents2[component] = 0.0;
   }

   myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(0.1, 55.0, inputComponents, lostComponents1); // with time Interval 1 = 0.1 Myr
   myBiodegrade.calculate(0.10005, 55.0, inputComponents, lostComponents2); // with time Interval 2 = 0.10005 Myr > time Interval 1 = 0.1 Myr

   for (component = 0; component < 13; ++component) // only the 13 first components with BioConst are interesting here
   {
      if (component != 11)  // no need to test on CO2 (should be equal and tested in another test)
         ASSERT_LT(lostComponents1[component], lostComponents2[component]);
   }


   // 5. Increasing the Biodegradation coefficients (FBi) should imply an decrease of the biodegradation
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents1[component] = 0.0;
      lostComponents2[component] = 0.0;
   }

   myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);   // with the original values { 0.1, 0.061, 0.05, 0.011, 0.007, 0.009, 0.001, 0.0008, 0.0003, 0.0005, 0.0008, 0.0, 0.001 }
   myBiodegrade.calculate(1, 55.0, inputComponents, lostComponents1);

   const double myconstBio2[] = { 0.11, 0.06100001, 0.06, 0.0111, 0.07, 0.01, 0.002, 0.000800001, 0.00035, 1, 0.008, 0.1, 10 };
   std::vector<double> constBio(myconstBio2, myconstBio2 + sizeof(myconstBio2) / sizeof(double));
   bioConsts2 = BioConsts(70.0, constBio);
   myBiodegrade2 = Biodegrade(80.0, bioConsts2, 0.5);   // with the new values { 0.11, 0.06100001, 0.06, 0.0111, 0.07, 0.01, 0.002, 0.000800001, 0.00035, 1, 0.008, 0.1, 10 }
   myBiodegrade2.calculate(1, 55.0, inputComponents, lostComponents2);

   for (component = 0; component < 13; ++component) // only the 13 first components with BioConst are interesting here
   {
      if (component != 11)  // no need to test on CO2 (should be equal and tested in another test)
         ASSERT_GT(lostComponents1[component], lostComponents2[component]);
   }
}


// This test evaluates the out-range values: no mass for the input components
TEST(Biodegrade, biodegradation_computation_no_input_mass)
{
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 0.0;
      lostComponents[component] = 0.0;
   }
   
   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(1.0, 65, inputComponents, lostComponents);

   for (component = 0; component < NumComponents; ++component)
   {
      EXPECT_DOUBLE_EQ(0.0, lostComponents[component]);
   }
   
}

// This test evaluates the out-range values: try to biodegrade more mass than what is inside the trap
TEST(Biodegrade, biodegradation_computation_overbiodegrade_boundary)
{
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100.0;
      lostComponents[component] = 0.0;
   }

   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 1);
   myBiodegrade.calculate(1.0, -10, inputComponents, lostComponents);

   for (component = 0; component < 13; ++component) // only the 13 first components with BioConst are interesting here
   {
      EXPECT_DOUBLE_EQ(100, lostComponents[component]);
   }

}


// Evaluation of the CO2 comportement: perfect biodegradation (for now)
TEST(Biodegrade, biodegradation_CO2_comportement)
{
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents[component] = 0.0;
   }

   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(0.000001, 79.9, inputComponents, lostComponents);
   EXPECT_DOUBLE_EQ(100.0, lostComponents[11]);

}


// This test evaluates: If the temperature of the trap is greater than the maximum temperature allowed for biodegradation, there should be no biodegradation
TEST(Biodegrade, biodegradation_Temperature_too_hot)
{
   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents[component] = 0.0;
   }
   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   myBiodegrade.calculate(1.0, 80.000001, inputComponents, lostComponents);

   for (component = 0; component < NumComponents; ++component)
   {
      EXPECT_DOUBLE_EQ(0.0, lostComponents[component]);
   }
}


// This test evaluates the comportement of the assert: assert(timeInterval > 0.0)
TEST(Biodegrade, biodegradation_death_test)
{
   ::testing::FLAGS_gtest_death_test_style = "threadsafe";

   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = 100;
      lostComponents[component] = 0.0;
   }

   // If the interval between the two snapshots is negative => assert
   Biodegrade myBiodegrade = Biodegrade(80.0, bioConsts, 0.5);
   ASSERT_DEATH(myBiodegrade.calculate(-1.0, 55.0, inputComponents, lostComponents), "Assertion.*timeInterval > 0.0");
}

} // namespace DataAccess::Interface
} // namespace DataAccess
