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
#include "../src/CompoundLithologyComposition.h"
#include "../src/GeoPhysicsProjectHandle.h"
#include "../src/GeoPhysicsObjectFactory.h"
#include "../src/LithologyManager.h"
#include "../src/MultiCompoundProperty.h"
#include "../src/CompoundProperty.h"
#include "../../utilities/src/FormattingException.h"
#include "../src/PermeabilityMixer.h"

#include <gtest/gtest.h>
#include <stdlib.h>

typedef formattingexception::GeneralException fastCauldronException;

using namespace GeoPhysics;


double* doubleAlloc ( const int numberOfItems ) {
   void* buf;
   int error = posix_memalign ( &buf, 32, sizeof ( double ) * numberOfItems );
   return static_cast<double*>(buf);
}

TEST(MixingPermeability, multiTest)
{

   ObjectFactory factory;
   ObjectFactory* factoryptr = &factory;
   ProjectHandle* projectHandle = dynamic_cast< ProjectHandle* >(OpenCauldronProject("MixingPermeabilityProject.project3d", "r", factoryptr));
   int NumberOflithologies;
   int NumberOfIterations;
   CompoundLithologyComposition composition;

   const unsigned int Size = 100;

   double* ves = doubleAlloc ( Size );
   double* maxVes = doubleAlloc ( Size );
   double* porosity = doubleAlloc ( Size );
   double* chemicalCompaction = doubleAlloc ( Size );
   double* porosityDerivative = doubleAlloc ( Size );
   double* permeabilityNormal = doubleAlloc ( Size );
   double* permeabilityPlane = doubleAlloc ( Size );
   double* permeabilityDerivative = doubleAlloc ( Size );
   CompoundProperty* mcps  = new CompoundProperty [ Size ];

   PermeabilityMixer::PermeabilityWorkSpaceArrays workSpace ( Size );

   double h = 1.0e7 / static_cast<double>(Size - 1);
   double v = 0.0;
   double mv = 0.0;

   for ( int i = 0; i < Size; ++i, mv += h ) {
      ves [ i ] = mv;
      maxVes [ i ] = mv;
      chemicalCompaction  [ i ] = 0.0;
   }

   for ( unsigned int whichLitho = 1; whichLitho <= 7; ++whichLitho ) {

      switch ( whichLitho ) {
        case 1: composition.setComposition ( "SM.Mudst.40%Clay" ,                 "",                 "", 100.0,  0.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 1; break;
        case 2: composition.setComposition ( "SM. Sandstone"    , "SM.Mudst.50%Clay", "SM.Mudst.60%Clay",  33.0, 33.0, 34.0, "Layered", 0.0 ); NumberOflithologies = 3; break;
        case 3: composition.setComposition ( "SM. Sandstone"    , "SM.Mudst.40%Clay", "SM.Mudst.50%Clay",  33.0, 33.0, 34.0, "Layered", 0.0 ); NumberOflithologies = 3; break;
        case 4: composition.setComposition ( "Std. Siltstone"   ,   "Std. Sandstone",       "Std. Shale",  33.0, 33.0, 34.0, "Layered", 0.0 ); NumberOflithologies = 3; break;
        case 5: composition.setComposition ( "SM.Mudst.60%Clay" ,                 "",                 "", 100.0,  0.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 1; break;
        case 6: composition.setComposition ( "SM.Mudst.50%Clay" , "SM.Mudst.40%Clay",                 "",  67.0, 33.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 2; break;
        case 7: composition.setComposition ( "Std. Sandstone"   ,   "Std. Sandstone",                 "",  67.0, 33.0,  0.0, "Layered", 0.0 ); NumberOflithologies = 2; break;

      default :
         throw fastCauldronException () << " Incorrect lithology selection";
      }

      CompoundLithology* myLitho = projectHandle->getLithologyManager ().getCompoundLithology ( composition );
      MultiCompoundProperty mcp ( NumberOflithologies, Size );

      for ( unsigned int j = 0; j < Size; ++j ) {
         myLitho->getPorosity ( ves [ j ], maxVes [ j ], false, chemicalCompaction [ j ], mcps [ j ]);

         mcp.getSimpleData ( 0, j ) = mcps [ j ] ( 0 );

         if ( NumberOflithologies > 1 ) {
            mcp.getSimpleData ( 1, j ) = mcps [ j ] ( 1 );
         }

         if ( NumberOflithologies > 2 ) {
            mcp.getSimpleData ( 2, j ) = mcps [ j ] ( 2 );
         }

         mcp.getMixedData ( j ) = mcps [ j ].mixedProperty ();
      }

      myLitho->calcBulkPermeabilityNP ( Size, ves, maxVes, mcp, permeabilityNormal, permeabilityPlane, workSpace );

      for ( unsigned int i = 0; i < Size; ++i ) {
         double permeabilityNormalScalar;
         double permeabilityPlaneScalar;

         myLitho->calcBulkPermeabilityNP ( ves [ i ], maxVes [ i ], mcps [ i ], permeabilityNormalScalar, permeabilityPlaneScalar );
         EXPECT_NEAR ( permeabilityNormalScalar, permeabilityNormal [ i ], 1.0e-12 );
         EXPECT_NEAR ( permeabilityPlaneScalar,  permeabilityPlane  [ i ], 1.0e-12 );
      }

   }


}

#if 1


TEST(MixingPermeability, layered)
{

   double permeabilityNormal;
   double permeabilityPlane;
   std::tr1::array<double,3> permVal  = { 1000.0, 50.0, 0.01 };
   std::tr1::array<double,3> permVal2 = { 1000.0, 50.0, 0.01 };

   std::vector<double> anisoVec ( { 1.0, 1.0, 1.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );

   PermeabilityMixer mixer;
   double layeringIndex;
   bool isFault = false;

   // Layering Index = 1.0
   layeringIndex = 1.0;
   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.029405771117828047, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 346.50340000000006, 1E-14);


   // Layering Index = 0.75
   layeringIndex = 0.75;
   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.050156865731445413, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 290.74014752595969, 1E-14);


   // Layering Index = 0.5
   layeringIndex = 0.5;
   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 0.20328466537757228, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 229.5897917682012, 1E-14);


   // Layering Index = 0.25
   layeringIndex = 0.25;
   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519342, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 163.91600641858543, 1E-14);


   // Layering Index = 0
   layeringIndex = 0.0;
   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 96.638199108923132, 1E-14);
   EXPECT_NEAR(permeabilityPlane,  96.638199108923175, 1E-14);

}

TEST(MixingPermeability, homogeneous)
{
   std::vector<double> anisoVec ( { 1.0, 1.0, 1.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );

   bool isFault = false;
   PermeabilityMixer mixer;
   double layeringIndex = -9999.0;

   //Homogeneous case
   double permeabilityNormal;
   double permeabilityPlane;
   std::tr1::array<double,3> permVal = { 1000.0, 50.0, 0.01 };
   std::tr1::array<double,3> permVal2 = { 1000.0, 50.0, 0.01 };

   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::HOMOGENEOUS, isFault );
   mixer.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

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
   PermeabilityMixer mixer;

   std::vector<double> anisoVec ( { 1.0, 1.0, 1.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );

   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 33);
   myLitho.addLithology(nullptr, 34);
   myLitho.makeFault(true);

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
   PermeabilityMixer mixer;
   bool isFault = true;
   double layeringIndex = 0.0;

   std::vector<double> anisoVec ( { 1.0, 1.0, 1.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );

   //Fault lithology -> homogeneous case by default
   double permeabilityNormal;
   double permeabilityPlane;
   std::tr1::array<double,3> permVal = { 1000.0, 50.0, 0.01 };
   std::tr1::array<double,3> permVal2 = { 1000.0, 50.0, 0.01 };

   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::HOMOGENEOUS, isFault );
   mixer.mixPermeability(permVal2, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519342, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 7.4243784490519342, 1E-14);
}

TEST(MixingPermeability, different_percentages)
{
   PermeabilityMixer mixer;
   bool isFault = false;
   double layeringIndex = 0.5;

   std::vector<double> anisoVec ( { 1.0, 1.0 });
   std::vector<double> percentVec ( { 100.0, 0.0 } );
   std::tr1::array<double,3> permVal12 = { 1000.0, 50.0, -99999 };

   double permeabilityNormal;
   double permeabilityPlane;

   //Effectively, only one lithology 100% 0% 0%
   std::tr1::array<double,3> permVal1 = { 1000.0, 50.0, -9999.0 };

   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal12, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 1000.0, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 1000.0, 1E-14);

   //Only two lithologies 90% 10% 0%

   std::tr1::array<double,3> permVal2 = { 1000.0, 50.0, -9999.0 };

   percentVec = { 90.0, 10.0 };

   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal12, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 622.06804487938086, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 873.20101826208634, 1E-14);

   //Three lithologies 70% 20% 10%

   std::tr1::array<double,3> permVal3 = { 1000.0, 50.0, 0.01 };
   std::tr1::array<double,3> permVal13 = { 1000.0, 50.0, 0.01 };

   anisoVec = { 1.0, 1.0, 1.0 };
   percentVec = { 70.0, 20.0, 10.0 };

   mixer.reset ( percentVec, anisoVec, false, layeringIndex, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal13, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 4.9076933527374793, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 620.11520100385258, 1E-14);

}


TEST(MixingPermeability, anisotropy)
{

   PermeabilityMixer mixer;
   bool isFault = false;


   double permeabilityNormal;
   double permeabilityPlane;

   std::tr1::array<double,3> permVal = { 1000.0, 50.0, 0.01 };
   std::tr1::array<double,3> permVal12 = { 1000.0, 50.0, 0.01 };
   // Anisotropy 0.1 0.1 0.1
   std::vector<double> anisoVec ( { 0.1, 0.1, 0.1 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );

   mixer.reset ( percentVec, anisoVec, false, 1.0, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal12, permeabilityNormal, permeabilityPlane);


   EXPECT_NEAR(permeabilityNormal, 0.029405771117828047, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 34.650340000000007, 1E-14);

   // Anisotropy 1 0.1 0.5
   anisoVec = { 1.0, 0.1, 0.5 };
   mixer.reset ( percentVec, anisoVec, false, 0.5, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeability(permVal12, permeabilityNormal, permeabilityPlane);


   EXPECT_NEAR(permeabilityNormal, 0.20328466537757228, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 198.03258557588396, 1E-14);

   // Anisotropy 1 0.1 0.5
   anisoVec = { 1.0, 0.1, 0.5 };
   mixer.reset ( percentVec, anisoVec, false, 1.0, DataAccess::Interface::HOMOGENEOUS, isFault );
   mixer.mixPermeability(permVal12, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519351, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 2.7435311965820768, 1E-14);

   // Anisotropy 0 0 0 -> Failure
   anisoVec = { 0.0, 0.0, 0.0 };
   mixer.reset ( percentVec, anisoVec, false, 1.0, DataAccess::Interface::HOMOGENEOUS, isFault );
   mixer.mixPermeability(permVal12, permeabilityNormal, permeabilityPlane);

   EXPECT_NEAR(permeabilityNormal, 7.4243784490519351, 1E-14);
   EXPECT_NEAR(permeabilityPlane, 0.0, 1E-14);

}

TEST(MixingPermeability, layeredDerivative)
{
   PermeabilityMixer mixer;
   bool isFault = false;

   double permeabilityDerivativeNormal=0;
   double permeabilityDerivativePlane=0;
   std::tr1::array<double,3> permVal = { 1000.0, 50.0, 0.01 };
   std::vector<double> anisoVec ( { 1.0, 1.0, 1.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );

   // Layering Index = 1.0
   std::tr1::array<double,3> permDerivativeVal1 ({ -100.0, -1000.0, -10000.0 });

   mixer.reset ( percentVec, anisoVec, false, 1.0, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeabilityDerivatives (permVal, permDerivativeVal1, permeabilityDerivativeNormal, permeabilityDerivativePlane );

   EXPECT_NEAR(permeabilityDerivativeNormal, -2.9015326777039680e-011, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, -3.7137923779000009e-012, 1E-20);

   // Layering Index = 0.75
   std::tr1::array<double,3> permDerivativeVal2 ({ 1.0, 10.0, 100.0 });

   mixer.reset ( percentVec, anisoVec, false, 0.75, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeabilityDerivatives (permVal, permDerivativeVal2, permeabilityDerivativeNormal, permeabilityDerivativePlane );

   EXPECT_NEAR(permeabilityDerivativeNormal,4.9315071216575015e-13, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 1.9070393490911652e-13, 1E-20);

   // Layering Index = 0.5
   std::tr1::array<double,3> permDerivativeVal3 ({ 10.0, 100.0, 1000.0 });

   mixer.reset ( percentVec, anisoVec, false, 0.5, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeabilityDerivatives (permVal, permDerivativeVal3, permeabilityDerivativeNormal, permeabilityDerivativePlane );

   EXPECT_NEAR(permeabilityDerivativeNormal, 1.8616683073217498e-11, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 9.5931574002593401e-12, 1E-20);

   // Layering Index = 0.25
   std::tr1::array<double,3> permDerivativeVal4 ({ 100.0, 1000.0, 10000.0 });

   mixer.reset ( percentVec, anisoVec, false, 0.25, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeabilityDerivatives (permVal, permDerivativeVal4, permeabilityDerivativeNormal, permeabilityDerivativePlane );

   EXPECT_NEAR(permeabilityDerivativeNormal, 2.4913279089200164e-009, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 4.3021150943305947e-010, 1E-20);

   // Layering Index = 0
   std::tr1::array<double,3> permDerivativeVal5 ({ 1000.0, 10000.0, 100000.0 });

   mixer.reset ( percentVec, anisoVec, false, 0.0, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeabilityDerivatives (permVal, permDerivativeVal5, permeabilityDerivativeNormal, permeabilityDerivativePlane );

   EXPECT_NEAR(permeabilityDerivativeNormal, 1.5229101977684013e-08, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane,  1.522910197768398e-08, 1E-20);

   // 0.0 derivative
   std::tr1::array<double,3> permDerivativeVal0 ({ 0.0, 0.0, 0.0 });

   mixer.reset ( percentVec, anisoVec, false, 1.0, DataAccess::Interface::LAYERED, isFault );
   mixer.mixPermeabilityDerivatives (permVal, permDerivativeVal0, permeabilityDerivativeNormal, permeabilityDerivativePlane );

   EXPECT_NEAR(permeabilityDerivativeNormal, 0.0, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 0.0, 1E-20);

}

TEST(MixingPermeability, homogeneousDerivative)
{

   PermeabilityMixer mixer;
   bool isFault = false;

   double permeabilityDerivativeNormal = 0;
   double permeabilityDerivativePlane = 0;

   std::vector<double> anisoVec ( { 1.0, 1.0, 1.0 });
   std::vector<double> percentVec ( { 33.0, 33.0, 34.0 } );
   std::tr1::array<double,3> permVal ({ 1000.0, 50.0, 0.01 });
   std::tr1::array<double,3> permDerivativeVal1 ({ 1.0, 10.0, 100.0 });

   // Layering Index = 1.0
   mixer.reset ( percentVec, anisoVec, false, 1.0, DataAccess::Interface::HOMOGENEOUS, isFault );
   mixer.mixPermeabilityDerivatives (permVal, permDerivativeVal1, permeabilityDerivativeNormal, permeabilityDerivativePlane );

   EXPECT_NEAR(permeabilityDerivativeNormal, 2.4913279089200166e-011, 1E-20);
   EXPECT_NEAR(permeabilityDerivativePlane, 2.4913279089200166e-011, 1E-20);

}

#endif
