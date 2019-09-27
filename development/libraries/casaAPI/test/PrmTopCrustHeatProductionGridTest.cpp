#include "../src/PrmTopCrustHeatProductionGrid.h"

#include "../src/ScenarioAnalysis.h"
#include <gtest/gtest.h>

TEST(PrmTopCrustHeatProductionGridTest, testConstructionFromBaseModel)
{
  casa::ScenarioAnalysis scenario;
  scenario.defineBaseCase( "Ottoland.project3d" );
  mbapi::Model& baseModel = scenario.baseCase();

  casa::PrmTopCrustHeatProductionGrid prm( baseModel, {} );
  ASSERT_EQ(ErrorHandler::ValidationError, baseModel.errorCode());
}
