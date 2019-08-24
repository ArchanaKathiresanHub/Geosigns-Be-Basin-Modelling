

#include <gtest/gtest.h>

#include "RunOptionsModelConverter.h"


//cmbAPI
#include "RunOptionsManager.h"


using namespace mbapi;

TEST(RunOptionsModelConverter, upgrade)
{
   Prograde::RunOptionsModelConverter modelConverter;
   std::string vreAlgorithm("Lopatin & Royden");
   std::string velAlgorithm("Gardner's Velocity-Density");
   std::string optimisationLevel1("Level 1");
   std::string optimisationLevel2("Level 2");
   std::string optimisationLevel3("Level 3");
   std::string optimisationLevel4("Level 4");
   std::string optimisationLevel5("Level 5");

   std::string PTCouplingMode("LooselyCoupled");
   int coupledMode = 0;

   std::string chemCompaction("Schneider");

   EXPECT_EQ("Burnham & Sweeney", modelConverter.upgradeVreAlgorithm(vreAlgorithm));
   EXPECT_EQ("Kennan's Velocity-Porosity", modelConverter.upgradeVelAlgorithm(velAlgorithm));

   // check optimisation levels
   EXPECT_EQ("Level 4", modelConverter.upgradeOptimisationLevel(optimisationLevel1));
   EXPECT_EQ("Level 4", modelConverter.upgradeOptimisationLevel(optimisationLevel2));
   EXPECT_EQ("Level 4", modelConverter.upgradeOptimisationLevel(optimisationLevel3));
   EXPECT_EQ("Level 4", modelConverter.upgradeOptimisationLevel(optimisationLevel4));
   EXPECT_EQ("Level 5", modelConverter.upgradeOptimisationLevel(optimisationLevel5));

   // coupling mode
   modelConverter.upgradePTCouplingMode(PTCouplingMode, coupledMode);
   EXPECT_EQ("ItCoupled", PTCouplingMode);
   EXPECT_EQ(1, coupledMode);

   // chem compaction
   EXPECT_EQ("Walderhaug", modelConverter.upgradeChemicalCompactionAlgorithm(chemCompaction));

   // temperature
   EXPECT_NEAR(0.0, modelConverter.upgradeTemperatureRange(-10.0, "dummy"), 1e-5);
   EXPECT_NEAR(1e5, modelConverter.upgradeTemperatureRange(100005, "dummy"), 1e-5);
   EXPECT_NEAR(2019, modelConverter.upgradeTemperatureRange(2019, "dummy"), 1e-5);


   // pressure
   EXPECT_NEAR(0.0, modelConverter.upgradePressureRange(-10.0, "dummy"), 1e-5);
   EXPECT_NEAR(1e5, modelConverter.upgradePressureRange(100005, "dummy"), 1e-5);
   EXPECT_NEAR(2019, modelConverter.upgradePressureRange(2019, "dummy"), 1e-5);



}
