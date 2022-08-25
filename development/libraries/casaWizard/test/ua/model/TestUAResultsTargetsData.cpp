//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/UAResultsTargetsData.h"

#include "model/predictionTargetDepth.h"
#include "model/predictionTargetSurface.h"
#include "DepthConverterStub.h"

#include <gtest/gtest.h>

using namespace casaWizard;
using namespace ua;

namespace
{

void compareTargets(const UAResultsTargetData& tableTarget, const PredictionTarget* predictionTarget)
{
   EXPECT_EQ(tableTarget.locationName,predictionTarget->locationName());
   EXPECT_EQ(tableTarget.locationName,predictionTarget->locationName());
   EXPECT_DOUBLE_EQ(tableTarget.x,predictionTarget->x());
   EXPECT_DOUBLE_EQ(tableTarget.y,predictionTarget->y());
   EXPECT_DOUBLE_EQ(tableTarget.z,predictionTarget->z());
   EXPECT_EQ(tableTarget.surfaceName,predictionTarget->surfaceName());
}

void compareTargetsVec(const UAResultsTargetsData& data, const QVector<const PredictionTarget*>& predictionTargets, const QVector<int>& expectedOrder)
{
   const QVector<UAResultsTargetData>& tableTargets = data.targetData();

   for(int i = 0; i < tableTargets.size(); i++)
   {
      compareTargets(tableTargets[i],predictionTargets[expectedOrder[i]]);
      EXPECT_EQ(tableTargets[i].targetIndex,expectedOrder[i]);
      EXPECT_EQ(data.targetIndex(i),expectedOrder[i]);
      EXPECT_EQ(data.rowIndex(expectedOrder[i]),i);
   }
}

void checkOrder(UAResultsTargetsData& data, const QVector<const PredictionTarget*>& predictionTargets, int sortIndex, QVector<int> expectedOrder)
{
   data.sortData(sortIndex);
   compareTargetsVec(data,predictionTargets,expectedOrder);
   data.sortData(sortIndex); //Sorting again with the sme column reverses the order
   std::reverse(expectedOrder.begin(),expectedOrder.end());
   compareTargetsVec(data,predictionTargets,expectedOrder);
}
}


TEST( TestUAResultsTargetsData, testSorting )
{
   DepthConverterStub depthConverter;

   PredictionTargetDepth depthTarget0({"Temperature","VRe"},10.0,19.0,30.0,100.0,"Ta");
   PredictionTargetDepth depthTarget1({"Temperature"},11.0,18.0,31.0,99.0,"ta");

   PredictionTargetSurface surfaceTarget0({"Temperature","VRe"},9.0,20.0,"surfaceName0",98.0,&depthConverter,"Tb");
   PredictionTargetSurface surfaceTarget1({"VRe"},8.0,21.0,"SurfaceName1",97.0,&depthConverter,"tb");

   QVector<const PredictionTarget*> targets{&depthTarget0,&depthTarget1,&surfaceTarget0,&surfaceTarget1};

   QVector<bool> hasTimeSeries(targets.size(),true);
   hasTimeSeries[1] = false;
   UAResultsTargetsData data(targets,{"Temperature","VRe"},hasTimeSeries);

   //Unsorted:
   QVector<int> expectedOrder{0,1,2,3};
   compareTargetsVec(data,targets,expectedOrder);

   //Initialization by setData
   QVector<const PredictionTarget*> targetsSetData{&depthTarget1,&depthTarget0,&surfaceTarget1,&surfaceTarget0};
   data.setData(targetsSetData,{"Temperature","VRe"},hasTimeSeries);
   expectedOrder = {0,1,2,3};
   compareTargetsVec(data,targetsSetData,expectedOrder);
   EXPECT_EQ(data.targetData().size(),4); //Targets should be replaced, not appended
   data.setData(targets,{"Temperature","VRe"},hasTimeSeries);

   checkOrder(data,targets,0,{0,2,1,3}); //Sort by name

   checkOrder(data,targets,1,{3,2,0,1}); //Sort by x
   EXPECT_EQ(data.tableRowsWithoutTimeSeries()[0],0); //Check order reverses the sort, hence why target 1 is expected at position 0

   checkOrder(data,targets,2,{1,0,2,3}); //Sort by y
   checkOrder(data,targets,3,{3,2,0,1}); //Sort by z
   checkOrder(data,targets,4,{1,0,3,2}); //Sort by surface
   checkOrder(data,targets,5,{2,0,1,3}); //Sort by property temperature
   checkOrder(data,targets,6,{3,0,2,1}); //Sort by property VRe
}
