#include "../src/PrmEquilibriumOceanicLithosphereThickness.h"

#include "../src/ScenarioAnalysis.h"
#include <gtest/gtest.h>

TEST(PrmEquilibriumOceanicLithosphereThicknessTest, testConstructionFromBaseModel)
{
  casa::ScenarioAnalysis scenario;
  scenario.defineBaseCase( "Ottoland.project3d" );
  mbapi::Model& baseModel = scenario.baseCase();

  casa::PrmEquilibriumOceanicLithosphereThickness prm( baseModel, {});

  // Test Constructor
  ASSERT_EQ(ErrorHandler::NoError, baseModel.errorCode());

  // Test asDoubleArray()
  EXPECT_DOUBLE_EQ(prm.asDoubleArray()[0], 92500.0);

  // Test == operator
  casa::PrmEquilibriumOceanicLithosphereThickness prm2( baseModel, {});
  ASSERT_EQ(ErrorHandler::NoError, baseModel.errorCode());
  EXPECT_TRUE( prm2 == prm );

  // Test 2nd constructor and the != operator
  casa::PrmEquilibriumOceanicLithosphereThickness prm3( 0, prm2, 1e5 );
  EXPECT_TRUE( prm2 != prm3 );
}
