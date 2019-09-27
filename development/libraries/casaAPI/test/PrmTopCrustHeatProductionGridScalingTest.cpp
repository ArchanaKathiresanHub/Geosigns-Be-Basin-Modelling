#include "../src/PrmTopCrustHeatProductionGridScaling.h"

#include "../src/ScenarioAnalysis.h"
#include <gtest/gtest.h>

TEST(PrmTopCrustHeatProductionGridScalingTest, testCaseWithoutMap)
{
  casa::ScenarioAnalysis scenario;
  scenario.defineBaseCase( "Ottoland.project3d" );
  mbapi::Model& baseModel = scenario.baseCase();

  casa::PrmTopCrustHeatProductionGridScaling prm( baseModel, {});
  ASSERT_EQ(ErrorHandler::ValidationError, baseModel.errorCode());
}
