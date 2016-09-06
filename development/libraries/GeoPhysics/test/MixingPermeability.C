//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/CompoundLithology.h"
#include "../src/GeoPhysicsProjectHandle.h"
#include "../src/GeoPhysicsObjectFactory.h"
#include "../../utilities/src/FormattingException.h"

#include <gtest/gtest.h>

typedef formattingexception::GeneralException fastCauldronException;

using namespace GeoPhysics;


TEST(MixingPermeability, layered)
{
   // Values for [Std.Sandstone, Std. Siltstone, Std Shale]
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast< ProjectHandle* >(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho (projectHandle);
     
   double permeabilityNormal;
   double permeabilityPlane;
   double permVal[3] = { 1000.0, 50.0, 0.01 };
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(false);
   myLitho.setPermAnisotropy(1, 1, 1);
   myLitho.setIsLegacy( false );
   
   // Layering Index = 1.0
   myLitho.setMixModel("Layered", 1.0 );
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.029405771117828047, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 346.50340000000006, 1E-14);


   // Layering Index = 0.75
   myLitho.setMixModel("Layered", 0.75);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.050156900050643084, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 290.74013483978803, 1E-14);


   // Layering Index = 0.5
   myLitho.setMixModel("Layered", 0.5);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.20328440266474265, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 229.58978928655733, 1E-14);


   // Layering Index = 0.25
   myLitho.setMixModel("Layered", 0.25);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519342, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 163.91600641858543, 1E-14);


   // Layering Index = 0
   myLitho.setMixModel("Layered", 0.0);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 96.638204738910616, 1E-14);
   EXPECT_NEAR(permeabilityPlane,  96.638196293929724, 1E-14);

}

TEST(MixingPermeability, homogeneous)
{
   // Values for [Std.Sandstone, Std. Siltstone, Std Shale]
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast<ProjectHandle*>(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho(projectHandle);

   //Homogeneous case
   double permeabilityNormal;
   double permeabilityPlane;
   double permVal[3] = { 1000.0, 50.0, 0.01 };
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(false);
   myLitho.setPermAnisotropy(1, 1, 1);

   myLitho.setMixModel("Homogeneous", -9999);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519342, 1E-14);
   EXPECT_NEAR(permeabilityPlane,  7.4243784490519342, 1E-14);

}

TEST(MixingPermeability, undefinedModel)
{
   //Undefined model
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast<ProjectHandle*>(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho(projectHandle);

   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(true);
   myLitho.setPermAnisotropy(1, 1, 1);

   try
   {
      myLitho.setMixModel("Blabla", -9999);
      FAIL() << "Expected 'MixModel not defined.' fastCauldronException";
   }
   catch (const fastCauldronException& ex)
   {
      //Check the correct exception is thrown
      EXPECT_EQ("MixModel not defined", std::string(ex.what()));
   }
   catch (...)
   {
      //If something else happen, then fail
      FAIL() << "MixModel not defined.' fastCauldronException";
   }

}

TEST(MixingPermeability, faultLithology)
{
   // Values for [Std.Sandstone, Std. Siltstone, Std Shale]
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast<ProjectHandle*>(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho(projectHandle);

   //Fault lithology -> homogeneous case by default
   double permeabilityNormal;
   double permeabilityPlane;
   double permVal[3] = { 1000.0, 50.0, 0.01 };
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(true);
   myLitho.setPermAnisotropy(1, 1, 1);

   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519342, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 7.4243784490519342, 1E-14);
}

TEST(MixingPermeability, different_percentages)
{
   // Values for [Std.Sandstone, Std. Siltstone]
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast<ProjectHandle*>(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho1(projectHandle);

   
   double permeabilityNormal;
   double permeabilityPlane;

   //Effectively, only one lithology 100% 0% 0% 
   double permVal1[2] = { 1000.0, 50.0 };
   myLitho1.addLithology(nullptr, 100);
   myLitho1.addLithology(nullptr, 0);
   myLitho1.makeFault(false);
   myLitho1.setPermAnisotropy(1, 1, 1);

   myLitho1.setMixModel("Layered", 0.5);
   myLitho1.reSetValuesForPermMixing();
   myLitho1.mixPermeability(permVal1, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 1000.0, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 1000.0, 1E-14);

   //Only two lithologies 90% 10% 0%
   CompoundLithology myLitho2(projectHandle);

   double permVal2[2] = { 1000.0, 50.0 };
   myLitho2.addLithology(nullptr, 90);
   myLitho2.addLithology(nullptr, 10);
   myLitho2.makeFault(false);
   myLitho2.setPermAnisotropy(1, 1, 1);

   myLitho2.setMixModel("Layered", 0.5);
   myLitho2.reSetValuesForPermMixing();
   myLitho2.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 622.06799340312455, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 873.20101710391941, 1E-14);

   //Three lithologies 70% 20% 10% 
   CompoundLithology myLitho3(projectHandle);

   double permVal3[3] = { 1000.0, 50.0, 0.01 };
   myLitho3.addLithology(nullptr, 70);
   myLitho3.addLithology(nullptr, 20);
   myLitho3.addLithology(nullptr, 10);
   myLitho3.makeFault(false);
   myLitho3.setPermAnisotropy(1, 1, 1);

   myLitho3.setMixModel("Layered", 0.5);
   myLitho3.reSetValuesForPermMixing();
   myLitho3.mixPermeability(permVal3, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 4.9076878781392184, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 620.11519892368403, 1E-14);

}

TEST(MixingPermeability, anisotropy)
{
   // Values for [Std.Sandstone, Std. Siltstone]
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast<ProjectHandle*>(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho(projectHandle);


   double permeabilityNormal;
   double permeabilityPlane;
  
   double permVal[3] = { 1000.0, 50.0, 0.01 };
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(false);

   // Anisotropy 0.1 0.1 0.1
   myLitho.setPermAnisotropy(0.1, 0.1, 0.1);

   myLitho.setMixModel("Layered", 1.0);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.029405771117828047, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 34.650340000000007, 1E-14);

   // Anisotropy 1 0.1 0.5
   myLitho.setPermAnisotropy(1, 0.1, 0.5);

   myLitho.setMixModel("Layered", 0.5);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.20328440266474265, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 198.03258466670627, 1E-14);
   
   // Anisotropy 1 0.1 0.5
   myLitho.setPermAnisotropy(1, 0.1, 0.5);

   myLitho.setMixModel("Homogeneous", -9998);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519351, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 2.7435311965820768, 1E-14);

   // Anisotropy 0 0 0 -> Failure
   myLitho.setPermAnisotropy(0, 0, 0);

   myLitho.setMixModel("Homogeneous", -9999);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeability(permVal, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519351, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 0.0, 1E-14);

}

TEST(MixingPermeability, layeredDerivative)
{
   // Values for [Std.Sandstone, Std. Siltstone, Std Shale]
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast< ProjectHandle* >(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho(projectHandle);

   double permeabilityDerivativeNormal=0;
   double permeabilityDerivativePlane=0;
   double permVal[3] = { 1000.0, 50.0, 0.01 };
   
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(false);
   myLitho.setPermAnisotropy(1, 1, 1);

   // Layering Index = 1.0
   double permDerivativeVal1[3] = { -100.0, -1000.0, -10000.0 };
   myLitho.setMixModel("Layered", 1.0);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeabilityDerivatives(permVal, permDerivativeVal1, permeabilityDerivativeNormal, permeabilityDerivativePlane);

   EXPECT_NEAR(permeabilityDerivativeNormal, -2.9015326777039680e-011, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, -3.7137923779000009e-012, 1E-20);

   // Layering Index = 0.75
   permeabilityDerivativeNormal = 0;
   permeabilityDerivativePlane = 0;
   double permDerivativeVal2[3] = { 1.0, 10.0, 100.0 };
   myLitho.setMixModel("Layered", 0.75);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeabilityDerivatives(permVal, permDerivativeVal2, permeabilityDerivativeNormal, permeabilityDerivativePlane);

   EXPECT_NEAR(permeabilityDerivativeNormal, 4.9315084589751653e-013, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 1.9070397604911900e-013, 1E-20);

   // Layering Index = 0.5
   permeabilityDerivativeNormal = 0;
   permeabilityDerivativePlane = 0;
   double permDerivativeVal3[3] = { 10.0, 100.0, 1000.0 };
   myLitho.setMixModel("Layered", 0.5);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeabilityDerivatives(permVal, permDerivativeVal3, permeabilityDerivativeNormal, permeabilityDerivativePlane);

   EXPECT_NEAR(permeabilityDerivativeNormal, 1.8616674841483343e-011, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 9.5931561197248072e-012, 1E-20);

   // Layering Index = 0.25
   permeabilityDerivativeNormal = 0;
   permeabilityDerivativePlane = 0;
   double permDerivativeVal4[3] = { 100.0, 1000.0, 10000.0 };
   myLitho.setMixModel("Layered", 0.25);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeabilityDerivatives(permVal, permDerivativeVal4, permeabilityDerivativeNormal, permeabilityDerivativePlane);

   EXPECT_NEAR(permeabilityDerivativeNormal, 2.4913279089200164e-009, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 4.3021150943305947e-010, 1E-20);

   // Layering Index = 0
   permeabilityDerivativeNormal = 0;
   permeabilityDerivativePlane = 0;
   double permDerivativeVal5[3] = { 1000.0, 10000.0, 100000.0 };
   myLitho.setMixModel("Layered", 0.0);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeabilityDerivatives(permVal, permDerivativeVal5, permeabilityDerivativeNormal, permeabilityDerivativePlane);

   EXPECT_NEAR(permeabilityDerivativeNormal, 1.5229090426731439e-008, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 1.5229103603974345e-008, 1E-20);

   // 0.0 derivative
   double permDerivativeVal0[3] = { 0.0, 0.0, 0.0 };
   myLitho.setMixModel("Layered", 1.0);
   myLitho.reSetValuesForPermMixing();
   myLitho.mixPermeabilityDerivatives(permVal, permDerivativeVal0, permeabilityDerivativeNormal, permeabilityDerivativePlane);

   EXPECT_NEAR(permeabilityDerivativeNormal, 0.0, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 0.0, 1E-20);

}

TEST(MixingPermeability, homogeneousDerivative)
{
   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast<ProjectHandle*>(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   CompoundLithology myLitho(projectHandle);

   double permeabilityDerivativeNormal = 0;
   double permeabilityDerivativePlane = 0;
   double permVal[3] = { 1000.0, 50.0, 0.01 };

   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(false);
   myLitho.setPermAnisotropy(1, 1, 1);

   // Layering Index = 1.0
   double permDerivativeVal1[3] = { 1.0, 10.0, 100.0 };
   myLitho.setMixModel("Homogeneous", -9999);
   myLitho.mixPermeabilityDerivatives(permVal, permDerivativeVal1, permeabilityDerivativeNormal, permeabilityDerivativePlane);

   EXPECT_NEAR(permeabilityDerivativeNormal, 2.4913279089200166e-011, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 2.4913279089200166e-011, 1E-20);

}