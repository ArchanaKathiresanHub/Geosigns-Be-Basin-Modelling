#include <gtest/gtest.h>

#include "ProjectIoModelConverter.h"


//cmbAPI
#include "ProjectDataManager.h"


using namespace mbapi;

TEST(ProjectIoModelConverter, upgrade)
{
   Prograde::ProjectIoModelConverter modelConverter;

   //tests to verify whether grid spacings are manipulated correctly if out of range values are found for node counts 
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaX("1d", 100.0, 2));//default value is found...nothing to do
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaX("1d", 500.0, 2));//default value is not found...upgraded to default value
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaX("Both", 500.0, 2));//default value is not found...upgraded to default value
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("1d", 500.0, 1));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("Both", 500.0, 1));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("3d", 500.0, 185));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("UnknownMode", 500.0, 185));

   EXPECT_EQ(100.0, modelConverter.upgradeDeltaY("1d", 100.0, 2));//default value is found...nothing to do
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaY("1d", 500.0, 2));//default value is not found...upgraded to default value
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaY("Both", 500.0, 2));//default value is not found...upgraded to default value
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("1d", 500.0, 1));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("Both", 500.0, 1));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("3d", 500.0, 125));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("UnknownMode", 500.0, 125));

   //tests to verify whether the description field is correctly manipulated
   EXPECT_EQ("Migrated from BPA", modelConverter.upgradeDescription("3d","")); 
   EXPECT_EQ("Migrated from BPA", modelConverter.upgradeDescription("1d", ""));
   EXPECT_EQ("Migrated from BPA (Please note that the original modelling mode was 1Dand3D)", modelConverter.upgradeDescription("Both", ""));
   EXPECT_EQ("Migrated from BPA", modelConverter.upgradeDescription("UnknownModellingMode", ""));
   EXPECT_EQ("TestProject NVG: Migrated from BPA", modelConverter.upgradeDescription("3d","TestProject NVG"));
   EXPECT_EQ("TestProject NVG: Migrated from BPA", modelConverter.upgradeDescription("1d", "TestProject NVG"));
   EXPECT_EQ("TestProject NVG: Migrated from BPA (Please note that the original modelling mode was 1Dand3D)", modelConverter.upgradeDescription("Both", "TestProject NVG"));
   EXPECT_EQ("TestProject NVG: Migrated from BPA", modelConverter.upgradeDescription("UnknownModellingMode", "TestProject NVG"));

   //Upgrading the simulation window 
   int legacyWindowXMin, legacyWindowXMax, legacyWindowYMin, legacyWindowYMax, stepX, stepY, xnodes, ynodes;
   //Case1: original window size 2 (not acceptable in BPA2) both in x-y direction. Udge cases in x-y direction
   xnodes = 185;
   ynodes = 125;
   legacyWindowXMin = 0;
   legacyWindowXMax = 1;
   legacyWindowYMin = 123;
   legacyWindowYMax = 124;
   stepX = 3;
   stepY = 3;
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(6, legacyWindowXMax);
   EXPECT_EQ(3, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(118, legacyWindowYMin);
   EXPECT_EQ(124, legacyWindowYMax);
   EXPECT_EQ(3, stepY);

   //Case2: original window size 2 (not acceptable in BPA2) both in x-y direction but with differnt subsampling size as compared to case-1. Udge cases in x-y direction
   xnodes = 185;
   ynodes = 125;
   legacyWindowXMin = 0;
   legacyWindowXMax = 1;
   legacyWindowYMin = 123;
   legacyWindowYMax = 124;
   stepX = 4;
   stepY = 4;
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(8, legacyWindowXMax);
   EXPECT_EQ(4, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(116, legacyWindowYMin);
   EXPECT_EQ(124, legacyWindowYMax);
   EXPECT_EQ(4, stepY);

   //Case3: original window size 1 (not acceptable in BPA2) both in x-y direction....edge of the domain
   xnodes = 185;
   ynodes = 125;
   legacyWindowXMin = 0;
   legacyWindowXMax = 0;
   legacyWindowYMin = 124;
   legacyWindowYMax = 124;
   stepX = 3;
   stepY = 3;
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(6, legacyWindowXMax);
   EXPECT_EQ(3, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(118, legacyWindowYMin);
   EXPECT_EQ(124, legacyWindowYMax);
   EXPECT_EQ(3, stepY);

   //Case4: original window size 1 (not acceptable in BPA2) both in x-y direction but with different sunsampling size as case-3....edge of the domain
   xnodes = 185;
   ynodes = 125;
   legacyWindowXMin = 0;
   legacyWindowXMax = 0;
   legacyWindowYMin = 124;
   legacyWindowYMax = 124;
   stepX = 4;
   stepY = 4;
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(8, legacyWindowXMax);
   EXPECT_EQ(4, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(116, legacyWindowYMin);
   EXPECT_EQ(124, legacyWindowYMax);
   EXPECT_EQ(4, stepY);

   //Case5: original windowing >= 3 in x (which is acceptable in BPA2)..but 2 in y (not acceptable) .incompatible combination of stepX-Y with respect to node counts and/or window area
   xnodes = 4;
   ynodes = 4;
   legacyWindowXMin = 0;
   legacyWindowXMax = 3;
   legacyWindowYMin = 0;
   legacyWindowYMax = 1;
   stepX = 3;
   stepY = 3;
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(3, legacyWindowXMax);
   EXPECT_EQ(1, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(0, legacyWindowYMin);
   EXPECT_EQ(2, legacyWindowYMax);
   EXPECT_EQ(1, stepY);

   //Case6: original window size 2 in both x-y-direction (not acceptable in BPA2)....incompatible stepX with respect to node counts
   xnodes = 5;
   ynodes = 5;
   legacyWindowXMin = 1;
   legacyWindowXMax = 2;
   legacyWindowYMin = 3;
   legacyWindowYMax = 4;
   stepX = 3;
   stepY = 3;
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(1, legacyWindowXMin);
   EXPECT_EQ(3, legacyWindowXMax);
   EXPECT_EQ(1, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(2, legacyWindowYMin);
   EXPECT_EQ(4, legacyWindowYMax);
   EXPECT_EQ(1, stepY);

   //Case7: original window size 2 in x-direction and 2 in y-direction (not acceptable in BPA2)....incompatible stepX with respect to node counts
   xnodes = 5;
   ynodes = 3;
   legacyWindowXMin = 2;
   legacyWindowXMax = 3;
   legacyWindowYMin = 1;
   legacyWindowYMax = 2;
   stepX = 3;
   stepY = 3;
   modelConverter.preProcessSimulationWindow(legacyWindowXMin, legacyWindowXMax, xnodes);//No modification in the inputs is needed
   EXPECT_EQ(2, legacyWindowXMin);
   EXPECT_EQ(3, legacyWindowXMax);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(2, legacyWindowXMin);
   EXPECT_EQ(4, legacyWindowXMax);
   EXPECT_EQ(1, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(0, legacyWindowYMin);
   EXPECT_EQ(2, legacyWindowYMax);
   EXPECT_EQ(1, stepY);

   //Case8: original window values are eronious. needed updation in the preprpcess step itself...incompatible stepX with respect to node counts
   xnodes = 185;
   ynodes = 125;
   legacyWindowXMin = -1;//not allowed (< 0)
   legacyWindowXMax = 185;//not allowed (>184)
   legacyWindowYMin = 10;
   legacyWindowYMax = 125;//not allowed (>124)
   stepX = 4;
   stepY = 4;
   modelConverter.preProcessSimulationWindow(legacyWindowXMin, legacyWindowXMax, xnodes);//Values will be modified
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(184, legacyWindowXMax);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(184, legacyWindowXMax);
   EXPECT_EQ(4, stepX);
   modelConverter.preProcessSimulationWindow(legacyWindowYMin, legacyWindowYMax, ynodes);
   EXPECT_EQ(10, legacyWindowYMin);
   EXPECT_EQ(124, legacyWindowYMax);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(10, legacyWindowYMin);
   EXPECT_EQ(124, legacyWindowYMax);
   EXPECT_EQ(4, stepY);

   //Case9: original window size 2 (not acceptable in BPA2) both in x-y direction. Window needs to be extended..no changes to step size
   xnodes = 185;
   ynodes = 125;
   legacyWindowXMin = 0;
   legacyWindowXMax = 1;
   legacyWindowYMin = 123;
   legacyWindowYMax = 124;
   stepX = 1;
   stepY = 1;
   modelConverter.upgradeSimulationWindow("Both", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(2, legacyWindowXMax);
   EXPECT_EQ(1, stepX);
   modelConverter.upgradeSimulationWindow("3d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(122, legacyWindowYMin);
   EXPECT_EQ(124, legacyWindowYMax);
   EXPECT_EQ(1, stepY);

   //Case10: A scenario with "Both" modelling mode but the inputs resembles to a 1d scenario...Such scenarios are found in BPA1 DB...
   //No updation is needed for such scenarios as it has been decided to treat these scenarios as 1d by the IMPORT. 
   xnodes = 2;
   legacyWindowXMin = 0;
   legacyWindowXMax = 1;
   stepX = 1;
   modelConverter.preProcessSimulationWindow(legacyWindowXMin, legacyWindowXMax, xnodes);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(1, legacyWindowXMax);
   modelConverter.upgradeSimulationWindow("Both", legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   EXPECT_EQ(0, legacyWindowXMin);
   EXPECT_EQ(1, legacyWindowXMax);
   EXPECT_EQ(1, stepX);

   //Case11: Similar to Case10 but a realistic "1d" scenario.. 
   ynodes = 2;
   legacyWindowYMin = 0;
   legacyWindowYMax = 1;
   stepY = 1; 
   modelConverter.upgradeSimulationWindow("1d", legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   EXPECT_EQ(0, legacyWindowYMin);
   EXPECT_EQ(1, legacyWindowYMax);
   EXPECT_EQ(1, stepY);
}
