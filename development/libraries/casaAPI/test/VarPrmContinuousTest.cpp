#include <gtest/gtest.h>

#include "../src/VarPrmContinuous.h"
#include "../src/PrmTopCrustHeatProduction.h"

#include "cmbAPI.h"
#include "casaAPI.h"

#include "RunCase.h"
#include "RunCaseImpl.h"

#include "MapInterpolatorNearestNeighbor.h"

using namespace casa;

class VarPrmContinuousTest : public ::testing::Test
{
public:
   VarPrmContinuousTest() :
      m_model{}
   {
      m_model.loadModelFromProjectFile("./VarPrmContinuousTest.project3d");
   }

   mbapi::Model m_model;
};

TEST_F( VarPrmContinuousTest, TestGenerateThreeDFromOneD_TopCrustHeatProduction )
{
   EXPECT_EQ(m_model.tableValueAsString("GridMapIoTbl", 0, "MapName"), "HeatMap");
   int tableSizeBeforeReplacingMap = m_model.tableSize("BasementIoTbl");
   EXPECT_EQ(tableSizeBeforeReplacingMap, 1);

   const casa::PrmTopCrustHeatProduction baseParameter(m_model, {});
   const std::vector<double> xin = {1, 2, 3};
   const std::vector<double> yin = {1, 2, 3};

   std::vector<SharedParameterPtr> parameters;
   SharedParameterPtr p1;
   p1.reset(new casa::PrmTopCrustHeatProduction(m_model, {}));
   SharedParameterPtr p2;
   p2.reset(new casa::PrmTopCrustHeatProduction(m_model, {}));
   SharedParameterPtr p3;
   p3.reset(new casa::PrmTopCrustHeatProduction(m_model, {}));

   parameters.push_back(p1);
   parameters.push_back(p2);
   parameters.push_back(p3);

   MapInterpolatorNearestNeighbor interpolator;
   casa::VarPrmContinuous* varPrm = new casa::VarPrmContinuousTemplate<casa::PrmTopCrustHeatProduction>(baseParameter, "name", 0, 5);
   varPrm->makeThreeDFromOneD(m_model, xin, yin, parameters, InterpolationParams(), interpolator);

   TableInfo tableInfo = baseParameter.tableInfo();

   int tableSizeAfterReplacingMap = m_model.tableSize("BasementIoTbl");
   EXPECT_EQ(tableSizeBeforeReplacingMap, tableSizeAfterReplacingMap); // Number of maps should stay the same since the old is replaced with the new
   EXPECT_EQ(m_model.tableValueAsString("GridMapIoTbl", 0, "MapName"), "Interpolated_TopCrustHeatProdGrid_Map");
   EXPECT_EQ(m_model.tableValueAsString(tableInfo.tableName, tableInfo.tableRow, tableInfo.variableGridName), "Interpolated_TopCrustHeatProdGrid_Map");
}

