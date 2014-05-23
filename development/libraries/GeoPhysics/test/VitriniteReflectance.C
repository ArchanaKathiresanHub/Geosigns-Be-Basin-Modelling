/** In this test case we check the behaviour of the VitriniteReflectance
    Class. The only method defined in the ../src/VitriniteReflectance.C
	file is create( const std::string & algorithmName ) so we are checking
	the pointer it returns for a range of "algorithmName" inputs.           */


#include "../src/VitriniteReflectance.h"
#include "../src/VreArrheniusMethod.h"
#include "../src/VreTtiMethod.h"

#include <iostream>
#include <cstring>
#include <cmath>

#include <gtest/gtest.h>


TEST( VitriniteReflectance, testing_create )
{
   using namespace GeoPhysics;

   /// Testing that the returned object is of the expected type
   
   /// So, for example, the "VRALGSWEENEYBURNHAM" string is expected to return a pointer to a SweeneyBurnham-type object
   SweeneyBurnham * my_Object1 = dynamic_cast<SweeneyBurnham*>(VitriniteReflectance::create("Burnham & Sweeney"));
   EXPECT_TRUE(my_Object1 != NULL);

   /// ... and so on
   Larter * my_Object2 = dynamic_cast<Larter*>(VitriniteReflectance::create("Larter"));
   EXPECT_TRUE(my_Object2 != NULL);

   VreTtiMethod * my_Object3 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Modified Lopatin"));
   EXPECT_TRUE(my_Object3 != NULL);

   VreTtiMethod * my_Object4 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));
   EXPECT_TRUE(my_Object4 != NULL);

   VreTtiMethod * my_Object5 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Wapples"));
   EXPECT_TRUE(my_Object5 != NULL);

   VreTtiMethod * my_Object6 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Royden"));
   EXPECT_TRUE(my_Object6 != NULL);

   VreTtiMethod * my_Object7 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Issler"));
   EXPECT_TRUE(my_Object7 != NULL);

   VreTtiMethod * my_Object8 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Hood"));
   EXPECT_TRUE(my_Object8 != NULL);

   VreTtiMethod * my_Object9 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Goff"));
   EXPECT_TRUE(my_Object9 != NULL);


   /// Testing that the returned pointer is NULL, i.e. object is NOT of a type other than expected

   /// So, for example, the "VRALGLARTER" string should not return a pointer to a VreTtiMethod-type object...
   VreTtiMethod * my_Object10 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Larter"));
   EXPECT_TRUE(my_Object10 == NULL);

   /// ... or a SweeneyBurnham-type object
   SweeneyBurnham * my_Object11 = dynamic_cast<SweeneyBurnham*>(VitriniteReflectance::create("Larter"));
   EXPECT_TRUE(my_Object11 == NULL);

   /// ... and so on
   Larter * my_Object12 = dynamic_cast<Larter*>(VitriniteReflectance::create("Burnham & Sweeney"));
   EXPECT_TRUE(my_Object12 == NULL);

   VreTtiMethod * my_Object13 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Burnham & Sweeney"));
   EXPECT_TRUE(my_Object13 == NULL);

   Larter * my_Object14 = dynamic_cast<Larter*>(VitriniteReflectance::create("Modified Lopatin"));
   EXPECT_TRUE(my_Object14 == NULL);

   SweeneyBurnham * my_Object15 = dynamic_cast<SweeneyBurnham*>(VitriniteReflectance::create("Modified Lopatin"));
   EXPECT_TRUE(my_Object15 == NULL);


   /// Testing that giberrish calls will also return NULL pointers
   
   VreTtiMethod * my_Object16 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("randomGibberish1"));
   EXPECT_TRUE(my_Object16 == NULL);

   SweeneyBurnham * my_Object17 = dynamic_cast<SweeneyBurnham*>(VitriniteReflectance::create("randomGibberish2"));
   EXPECT_TRUE(my_Object17 == NULL);

   Larter * my_Object18 = dynamic_cast<Larter*>(VitriniteReflectance::create("randomGibberish3"));
   EXPECT_TRUE(my_Object18 == NULL);
}
