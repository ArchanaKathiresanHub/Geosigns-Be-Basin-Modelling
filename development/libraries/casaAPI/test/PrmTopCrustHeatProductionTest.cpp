#include "../src/PrmTopCrustHeatProduction.h"

#include "../src/ScenarioAnalysis.h"
#include <gtest/gtest.h>

TEST(PrmTopCrustHeatProductionTest, testConstructionFromBaseModel)
{
  casa::ScenarioAnalysis scenario;
  scenario.defineBaseCase( "Ottoland.project3d" );
  mbapi::Model& baseModel = scenario.baseCase();

  casa::PrmTopCrustHeatProduction prm( baseModel, {});

  ASSERT_EQ(ErrorHandler::NoError, baseModel.errorCode());
}
