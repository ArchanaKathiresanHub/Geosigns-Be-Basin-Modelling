//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/CapillarySealStrength.h"
#include "ConstantsMathematics.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace migration;

// This test makes sure that for a wide range of seal permeabilities (always smaller than in the reservoir),
// the resulting capillary strength (i.e. \Delta P_c) is positive, for a simplistic composition and some
// standard choices for values of tempreature and pressure.
TEST(CapillarySealStrength, PositiveStrength)
{
   // Litho properties
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_res1  (1.0, 0.33, 0.2611);
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_seal1 (1e-5, 0.33, 0.2611);

   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_res;
   lithoProp_res.push_back(lithoProp_res1);
   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_seal;
   lithoProp_seal.push_back(lithoProp_seal1);

   std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> > lithoProps;
   lithoProps.push_back(lithoProp_res);
   lithoProps.push_back(lithoProp_seal);

   // Litho fractions
   double lithFrac_res1  = 1.0;
   double lithFrac_seal1 = 1.0;

   std::vector<double> lithFrac_res;
   lithFrac_res.push_back(lithFrac_res1);
   std::vector<double> lithFrac_seal;
   lithFrac_seal.push_back(lithFrac_seal1);

   std::vector< std::vector<double> > lithFracs;
   lithFracs.push_back(lithFrac_res);
   lithFracs.push_back(lithFrac_seal);

   // Mix models
   CBMGenerics::capillarySealStrength::MixModel mixModel_res  = CBMGenerics::capillarySealStrength::Homogeneous;
   CBMGenerics::capillarySealStrength::MixModel mixModel_seal = CBMGenerics::capillarySealStrength::Homogeneous;

   std::vector<CBMGenerics::capillarySealStrength::MixModel> mixModels;
   mixModels.push_back(mixModel_res);
   mixModels.push_back(mixModel_seal);

   for (int i = 1; i < 100; ++i)
   {
      // Permeabilities
      double permeability_res  = 1.0;
      // Permeability: [1e-9,1e-3]
      double permeability_seal = 1e-9 * (double) i * (double) i * (double) i;

      std::vector<double> permeabilities;
      permeabilities.push_back(permeability_res);
      permeabilities.push_back(permeability_seal);

      // Seal fluid density
      double sealFluidDensity = 1000.0;
   
      // Lambda correction in the reservoir
      double lambdaPc = 1.0;

      // non-legacy run
      bool legacy = false;

      // Put all the above parameters together
      CapillarySealStrength capSealStrength (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

      // Create the vapour and liquid compositions
      Composition vapourComposition;
      Composition liquidComposition;

      for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         vapourComposition.add ((ComponentId)componentId, 1.0);
         liquidComposition.add ((ComponentId)componentId, 1.0);
      }

      std::vector<Composition> composition;
      composition.push_back(vapourComposition);
      composition.push_back(liquidComposition);

      // GOR follows from the compositions
      double gorm = computeGorm (vapourComposition, liquidComposition);

      // Some typical values for temperature and pressure
      double temperature = 100.0 + Utilities::Maths::CelciusToKelvin;
      double pressure    = 20.0; // MPa

      // CapillarySealStrength for vapour and liquid
      double CapSealStrengthVapour, CapSealStrengthLiquid;

      // 1st round k_seal = 10^-9 mD

      // Where the magic happens
      capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

      EXPECT_GT(CapSealStrengthVapour, 0.0);
      EXPECT_GT(CapSealStrengthLiquid, 0.0);
}
}


// This test complements the first one, by setting relatively high permeability values in the seal.
// As a result the capSealStrength calculation returns negative values which are then set to 0 by default.
TEST(CapillarySealStrength, ZeroStrength)
{
   // Litho properties
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_res1  (1.0, 0.33, 0.2611);
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_seal1 (1e-5, 0.33, 0.2611);

   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_res;
   lithoProp_res.push_back(lithoProp_res1);
   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_seal;
   lithoProp_seal.push_back(lithoProp_seal1);

   std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> > lithoProps;
   lithoProps.push_back(lithoProp_res);
   lithoProps.push_back(lithoProp_seal);

   // Litho fractions
   double lithFrac_res1  = 1.0;
   double lithFrac_seal1 = 1.0;

   std::vector<double> lithFrac_res;
   lithFrac_res.push_back(lithFrac_res1);
   std::vector<double> lithFrac_seal;
   lithFrac_seal.push_back(lithFrac_seal1);

   std::vector< std::vector<double> > lithFracs;
   lithFracs.push_back(lithFrac_res);
   lithFracs.push_back(lithFrac_seal);

   // Mix models
   CBMGenerics::capillarySealStrength::MixModel mixModel_res  = CBMGenerics::capillarySealStrength::Homogeneous;
   CBMGenerics::capillarySealStrength::MixModel mixModel_seal = CBMGenerics::capillarySealStrength::Homogeneous;

   std::vector<CBMGenerics::capillarySealStrength::MixModel> mixModels;
   mixModels.push_back(mixModel_res);
   mixModels.push_back(mixModel_seal);

   for (int i = 1; i < 100; ++i)
   {
      // Permeabilities
      double permeability_res  = 1.0;
      double permeability_seal = 1e-2 * (double) i;

      std::vector<double> permeabilities;
      permeabilities.push_back(permeability_res);
      permeabilities.push_back(permeability_seal);

      // Seal fluid density
      double sealFluidDensity = 1000.0;
   
      // Lambda correction in the reservoir
      double lambdaPc = 2.0;

      // non-legacy run
      bool legacy = false;

      // Put all the above parameters together
      CapillarySealStrength capSealStrength (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

      // Create the vapour and liquid compositions
      Composition vapourComposition;
      Composition liquidComposition;

      for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         vapourComposition.add ((ComponentId)componentId, 1.0);
         liquidComposition.add ((ComponentId)componentId, 1.0);
      }

      std::vector<Composition> composition;
      composition.push_back(vapourComposition);
      composition.push_back(liquidComposition);

      // GOR follows from the compositions
      double gorm = computeGorm (vapourComposition, liquidComposition);

      // Some typical values for temperature and pressure
      double temperature = 100.0 + Utilities::Maths::CelciusToKelvin;
      double pressure    = 20.0; // MPa

      // CapillarySealStrength for vapour and liquid
      double CapSealStrengthVapour, CapSealStrengthLiquid;

      capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

      EXPECT_EQ(CapSealStrengthVapour, 0.0);
      EXPECT_EQ(CapSealStrengthLiquid, 0.0);
   }
}


// This tests makes sure that the capillary seal strength for vapour is higher when there is liquid
// HCs present in the trap, due to the wetting order: brine > liquid > vapour
TEST(CapillarySealStrength, VapourStrengthDependsOnOil)
{
   // Litho properties
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_res1  (1.0, 0.33, 0.2611);
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_seal1 (1e-5, 0.33, 0.2611);

   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_res;
   lithoProp_res.push_back(lithoProp_res1);
   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_seal;
   lithoProp_seal.push_back(lithoProp_seal1);

   std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> > lithoProps;
   lithoProps.push_back(lithoProp_res);
   lithoProps.push_back(lithoProp_seal);

   // Litho fractions
   double lithFrac_res1  = 1.0;
   double lithFrac_seal1 = 1.0;

   std::vector<double> lithFrac_res;
   lithFrac_res.push_back(lithFrac_res1);
   std::vector<double> lithFrac_seal;
   lithFrac_seal.push_back(lithFrac_seal1);

   std::vector< std::vector<double> > lithFracs;
   lithFracs.push_back(lithFrac_res);
   lithFracs.push_back(lithFrac_seal);

   // Mix models
   CBMGenerics::capillarySealStrength::MixModel mixModel_res  = CBMGenerics::capillarySealStrength::Homogeneous;
   CBMGenerics::capillarySealStrength::MixModel mixModel_seal = CBMGenerics::capillarySealStrength::Homogeneous;

   std::vector<CBMGenerics::capillarySealStrength::MixModel> mixModels;
   mixModels.push_back(mixModel_res);
   mixModels.push_back(mixModel_seal);
   
   for (int i = 0; i < 100; i = i+3)
   {
      for (int j = 1; j < 100; j = j+3)
      {
         for (int k = 1; k < 100; k = k+3)
         {

            // Permeabilities
            double permeability_res  = 1.0;
            double permeability_seal = 1e-9 * (10.0*(double)i + 1.0);

            std::vector<double> permeabilities;
            permeabilities.push_back(permeability_res);
            permeabilities.push_back(permeability_seal);

            // Seal fluid density
            double sealFluidDensity = 1000.0;
   
            // Lambda correction in the reservoir
            double lambdaPc = 1.0;

            // non-legacy run
            bool legacy = false;

            // Put all the above parameters together
            CapillarySealStrength capSealStrength (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

            // Create the vapour and liquid compositions
            Composition vapourComposition;
            Composition liquidComposition;

            for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
            {
               vapourComposition.set ((ComponentId)componentId, 0.0);
               liquidComposition.set ((ComponentId)componentId, 0.0);

               switch ((ComponentId)componentId)
               {
               case ComponentId::C1 :
               case ComponentId::C2 :
               case ComponentId::C3 :
               case ComponentId::C4 :
               case ComponentId::C5 :
               case ComponentId::H2S:
                  vapourComposition.add ((ComponentId)componentId, 1.0);
                  break;
               default :
                  liquidComposition.add ((ComponentId)componentId, 1.0);
                  break;
               }
            }

            std::vector<Composition> composition;
            composition.push_back(vapourComposition);
            composition.push_back(liquidComposition);

            // GOR follows from the compositions
            double gorm = computeGorm (vapourComposition, liquidComposition);

            // Some typical values for temperature and pressure
            double temperature = 10.0 * (double)j + Utilities::Maths::CelciusToKelvin;
            double pressure    = 0.1 + 5.0 * (double) k ; // MPa

            // CapillarySealStrength for vapour and liquid
            double CapSealStrengthVapour, CapSealStrengthLiquid;

            capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

            double vapourStrengthWithLiquid = CapSealStrengthVapour;

            Composition vapourComposition2, liquidComposition2;

            // Repeating calculations after removing the liquid phase from the trap
            for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
            {
               vapourComposition2.set ((ComponentId)componentId, 0.0);
               liquidComposition2.set ((ComponentId)componentId, 0.0);

               switch ((ComponentId)componentId)
               {
               case ComponentId::C1 :
               case ComponentId::C2 :
               case ComponentId::C3 :
               case ComponentId::C4 :
               case ComponentId::C5 :
               case ComponentId::H2S:
                  vapourComposition2.add ((ComponentId)componentId, 1.0);
                  break;
               default :
                  break;
               }
            }

            std::vector<Composition> composition2;
            composition2.push_back(vapourComposition2);
            composition2.push_back(liquidComposition2);

            double gorm2 = computeGorm (vapourComposition2, liquidComposition2);

            double CapSealStrengthVapour2, CapSealStrengthLiquid2;
            capSealStrength.compute(composition2, gorm2, temperature, pressure, CapSealStrengthVapour2, CapSealStrengthLiquid2);

            double vapourStrengthWithOutLiquid = CapSealStrengthVapour2;

            EXPECT_GT(vapourStrengthWithOutLiquid, vapourStrengthWithLiquid);
         }
      }
   }
}


// This test explores the influence of lambda on the results. When lambda is zero, the smalles difference in
// permeabilities translates directly into capillary pressures (i.e. the correction factor becomes one).
// In this test we consider vapour and liquid compositions and compare only the liquid (wetting order same in reservoir and seal).
TEST(CapillarySealStrength, LambdaInfluenceLiquid)
{
   // Litho properties
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_res1  (1.0, 0.33, 0.2611);
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_seal1 (1e-5, 0.33, 0.2611);

   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_res;
   lithoProp_res.push_back(lithoProp_res1);
   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_seal;
   lithoProp_seal.push_back(lithoProp_seal1);

   std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> > lithoProps;
   lithoProps.push_back(lithoProp_res);
   lithoProps.push_back(lithoProp_seal);

   // Litho fractions
   double lithFrac_res1  = 1.0;
   double lithFrac_seal1 = 1.0;

   std::vector<double> lithFrac_res;
   lithFrac_res.push_back(lithFrac_res1);
   std::vector<double> lithFrac_seal;
   lithFrac_seal.push_back(lithFrac_seal1);

   std::vector< std::vector<double> > lithFracs;
   lithFracs.push_back(lithFrac_res);
   lithFracs.push_back(lithFrac_seal);

   // Mix models
   CBMGenerics::capillarySealStrength::MixModel mixModel_res  = CBMGenerics::capillarySealStrength::Homogeneous;
   CBMGenerics::capillarySealStrength::MixModel mixModel_seal = CBMGenerics::capillarySealStrength::Homogeneous;

   std::vector<CBMGenerics::capillarySealStrength::MixModel> mixModels;
   mixModels.push_back(mixModel_res);
   mixModels.push_back(mixModel_seal);

   // Permeabilities
   double permeability_res  = 1.0;
   double permeability_seal = 1.0 - 0.0000000001;

   std::vector<double> permeabilities;
   permeabilities.push_back(permeability_res);
   permeabilities.push_back(permeability_seal);

   // Seal fluid density
   double sealFluidDensity = 1000.0;
   
   // Lambda is zero this time to make the correction factor equal to one.
   double lambdaPc = 0.0;

   // non-legacy run
   bool legacy = false;

   // Put all the above parameters together
   CapillarySealStrength capSealStrength (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

   // Create the vapour and liquid compositions
   Composition vapourComposition;
   Composition liquidComposition;

   for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
   {
      vapourComposition.add ((ComponentId)componentId, 1.0);
      liquidComposition.add ((ComponentId)componentId, 1.0);
   }

   std::vector<Composition> composition;
   composition.push_back(vapourComposition);
   composition.push_back(liquidComposition);

   // GOR follows from the compositions
   double gorm = computeGorm (vapourComposition, liquidComposition);

   // Some typical values for temperature and pressure
   double temperature = 100.0 + Utilities::Maths::CelciusToKelvin;
   double pressure    = 20.0; // MPa

   // CapillarySealStrength for vapour and liquid
   double CapSealStrengthVapour, CapSealStrengthLiquid;

   capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

   EXPECT_GT(CapSealStrengthLiquid, 0.0);

   // And now setting the two permeabilities equal to each other should make the strength zero
   permeability_seal = permeability_res;

   permeabilities.pop_back();
   permeabilities.push_back(permeability_seal);

   CapillarySealStrength capSealStrength2 (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

   capSealStrength2.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

   EXPECT_EQ(CapSealStrengthLiquid, 0.0);
}


// This test explores the influence of lambda on the results. When lambda is zero, the smalles difference in
// permeabilities translates directly into capillary pressures (i.e. the correction factor becomes one).
// In this test we consider vapour-only compositions in order to have the same wetting order in reservoir and seal.
TEST(CapillarySealStrength, LambdaInfluenceVapour)
{
   // Litho properties
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_res1  (1.0, 0.33, 0.2611);
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_seal1 (1e-5, 0.33, 0.2611);

   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_res;
   lithoProp_res.push_back(lithoProp_res1);
   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_seal;
   lithoProp_seal.push_back(lithoProp_seal1);

   std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> > lithoProps;
   lithoProps.push_back(lithoProp_res);
   lithoProps.push_back(lithoProp_seal);

   // Litho fractions
   double lithFrac_res1  = 1.0;
   double lithFrac_seal1 = 1.0;

   std::vector<double> lithFrac_res;
   lithFrac_res.push_back(lithFrac_res1);
   std::vector<double> lithFrac_seal;
   lithFrac_seal.push_back(lithFrac_seal1);

   std::vector< std::vector<double> > lithFracs;
   lithFracs.push_back(lithFrac_res);
   lithFracs.push_back(lithFrac_seal);

   // Mix models
   CBMGenerics::capillarySealStrength::MixModel mixModel_res  = CBMGenerics::capillarySealStrength::Homogeneous;
   CBMGenerics::capillarySealStrength::MixModel mixModel_seal = CBMGenerics::capillarySealStrength::Homogeneous;

   std::vector<CBMGenerics::capillarySealStrength::MixModel> mixModels;
   mixModels.push_back(mixModel_res);
   mixModels.push_back(mixModel_seal);

   // Permeabilities
   double permeability_res  = 1.0;
   double permeability_seal = 1.0 - 0.0000000001;

   std::vector<double> permeabilities;
   permeabilities.push_back(permeability_res);
   permeabilities.push_back(permeability_seal);

   // Seal fluid density
   double sealFluidDensity = 1000.0;
   
   // Lambda is zero this time to make the correction factor equal to one.
   double lambdaPc = 0.0;

   // non-legacy run
   bool legacy = false;

   // Put all the above parameters together
   CapillarySealStrength capSealStrength (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

   // Create the vapour and liquid compositions
   Composition vapourComposition;
   Composition liquidComposition;

   for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
   {
      switch ((ComponentId)componentId)
      {
      case ComponentId::C1 :
      case ComponentId::C2 :
      case ComponentId::C3 :
      case ComponentId::C4 :
      case ComponentId::C5 :
      case ComponentId::H2S:
         vapourComposition.add ((ComponentId)componentId, 1.0);
         break;
      default :
         break;
      }
   }

   std::vector<Composition> composition;
   composition.push_back(vapourComposition);
   composition.push_back(liquidComposition);

   // GOR follows from the compositions
   double gorm = computeGorm (vapourComposition, liquidComposition);

   // Some typical values for temperature and pressure
   double temperature = 100.0 + Utilities::Maths::CelciusToKelvin;
   double pressure    = 20.0; // MPa

   // CapillarySealStrength for vapour and liquid
   double CapSealStrengthVapour, CapSealStrengthLiquid;

   capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

   EXPECT_GT(CapSealStrengthVapour, 0.0);

   // And now setting the two permeabilities equal to each other should make the strength zero
   permeability_seal = permeability_res;

   permeabilities.pop_back();
   permeabilities.push_back(permeability_seal);

   CapillarySealStrength capSealStrength2 (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

   capSealStrength2.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

   EXPECT_EQ(CapSealStrengthVapour, 0.0);
}


// This test make sure that empty compositions result in zero strength.
TEST(CapillarySealStrength, EmptyCompositions)
{
   
// Litho properties
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_res1  (1.0, 0.33, 0.2611);
   CBMGenerics::capillarySealStrength::LithoProp lithoProp_seal1 (1e-5, 0.33, 0.2611);

   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_res;
   lithoProp_res.push_back(lithoProp_res1);
   std::vector<translateProps::CreateCapillaryLithoProp::output> lithoProp_seal;
   lithoProp_seal.push_back(lithoProp_seal1);

   std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> > lithoProps;
   lithoProps.push_back(lithoProp_res);
   lithoProps.push_back(lithoProp_seal);

   // Litho fractions
   double lithFrac_res1  = 1.0;
   double lithFrac_seal1 = 1.0;

   std::vector<double> lithFrac_res;
   lithFrac_res.push_back(lithFrac_res1);
   std::vector<double> lithFrac_seal;
   lithFrac_seal.push_back(lithFrac_seal1);

   std::vector< std::vector<double> > lithFracs;
   lithFracs.push_back(lithFrac_res);
   lithFracs.push_back(lithFrac_seal);

   // Mix models
   CBMGenerics::capillarySealStrength::MixModel mixModel_res  = CBMGenerics::capillarySealStrength::Homogeneous;
   CBMGenerics::capillarySealStrength::MixModel mixModel_seal = CBMGenerics::capillarySealStrength::Homogeneous;

   std::vector<CBMGenerics::capillarySealStrength::MixModel> mixModels;
   mixModels.push_back(mixModel_res);
   mixModels.push_back(mixModel_seal);

   // Permeabilities
   double permeability_res  = 1.0;
   double permeability_seal = 1e-9;

   std::vector<double> permeabilities;
   permeabilities.push_back(permeability_res);
   permeabilities.push_back(permeability_seal);

   // Seal fluid density
   double sealFluidDensity = 1000.0;
   
   // Lambda is zero this time to make the correction factor equal to one.
   double lambdaPc = 0.0;

   // non-legacy run
   bool legacy = false;

   // Put all the above parameters together
   CapillarySealStrength capSealStrength (lithoProps, lithFracs, mixModels, permeabilities, sealFluidDensity, lambdaPc, legacy);

   // Create the vapour and liquid compositions
   Composition vapourComposition;
   Composition liquidComposition;

   // Adding empty compositions
   std::vector<Composition> composition;
   composition.push_back(vapourComposition);
   composition.push_back(liquidComposition);

   // GOR follows from the compositions
   double gorm = computeGorm (vapourComposition, liquidComposition);

   // Some typical values for temperature and pressure
   double temperature = 100.0 + Utilities::Maths::CelciusToKelvin;
   double pressure    = 20.0; // MPa

   // CapillarySealStrength for vapour and liquid
   double CapSealStrengthVapour, CapSealStrengthLiquid;

   capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

   EXPECT_EQ(CapSealStrengthVapour, 0.0);
   EXPECT_EQ(CapSealStrengthLiquid, 0.0);

   for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
   {
      switch ((ComponentId)componentId)
      {
      case ComponentId::C1 :
      case ComponentId::C2 :
      case ComponentId::C3 :
      case ComponentId::C4 :
      case ComponentId::C5 :
      case ComponentId::H2S:
         vapourComposition.add ((ComponentId)componentId, 1.0);
         break;
      default :
         break;
      }
   }

   composition.pop_back();
   composition.pop_back();

   composition.push_back(vapourComposition);
   composition.push_back(liquidComposition);

   // GOR follows from the compositions
   gorm = computeGorm (vapourComposition, liquidComposition);

   capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

   EXPECT_EQ(CapSealStrengthLiquid, 0.0);

   for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
   {
      switch ((ComponentId)componentId)
      {
      case ComponentId::C1 :
      case ComponentId::C2 :
      case ComponentId::C3 :
      case ComponentId::C4 :
      case ComponentId::C5 :
      case ComponentId::H2S:
         vapourComposition.set ((ComponentId)componentId, 0.0);
         break;
      default :
         liquidComposition.add ((ComponentId)componentId, 1.0);
         break;
      }
   }

   composition.pop_back();
   composition.pop_back();

   composition.push_back(vapourComposition);
   composition.push_back(liquidComposition);

   // GOR follows from the compositions
   gorm = computeGorm (vapourComposition, liquidComposition);

   capSealStrength.compute(composition, gorm, temperature, pressure, CapSealStrengthVapour, CapSealStrengthLiquid);

   EXPECT_EQ(CapSealStrengthVapour, 0.0);
}
