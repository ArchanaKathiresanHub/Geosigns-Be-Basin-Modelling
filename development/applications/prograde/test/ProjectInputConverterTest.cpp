#include <gtest/gtest.h>

#include "ProjectIoModelConverter.h"


//cmbAPI
#include "ProjectDataManager.h"


using namespace mbapi;

TEST(ProjectIoModelConverter, upgrade)
{
   Prograde::ProjectIoModelConverter modelConverter;
   std::string modellingMode="1d";
   EXPECT_EQ("3d", modelConverter.upgradeModellingModeFor1D(modellingMode));
   EXPECT_EQ("3d", modelConverter.upgradeModellingModeFor1D("3d"));
   EXPECT_EQ("UnknownMode", modelConverter.upgradeModellingModeFor1D("UnknownMode"));

   double xcoord = 100.0, ycoord = 200.0, deltaX = 50.0, deltaY = 100.0;
   //tests to verify origin coordinates (x,y) are correctly manipulated for different modelling modes
   EXPECT_EQ(xcoord, modelConverter.upgradeProjectOriginX(modellingMode, xcoord, deltaX));
   EXPECT_EQ(xcoord, modelConverter.upgradeProjectOriginX("UnknownMode", xcoord, deltaX));
   EXPECT_EQ(75.0, modelConverter.upgradeProjectOriginX("3d", xcoord, deltaX));
   EXPECT_EQ(ycoord, modelConverter.upgradeProjectOriginX(modellingMode, ycoord, deltaY));
   EXPECT_EQ(ycoord, modelConverter.upgradeProjectOriginX("UnknownMode", ycoord, deltaY));
   EXPECT_EQ(150.0, modelConverter.upgradeProjectOriginX("3d", ycoord, deltaY));
   //tests to verify node counts are correctly manipulated if out of range values are found
   int nodeX = 1, nodeY = 2;
   EXPECT_EQ(2, modelConverter.upgradeNodeX("1d", nodeX));
   EXPECT_EQ(3, modelConverter.upgradeNodeX("3d", nodeX));
   EXPECT_EQ(nodeX, modelConverter.upgradeNodeX("UnknownMode", nodeX));

   EXPECT_EQ(2, modelConverter.upgradeNodeY("1d", nodeY)); 
   EXPECT_EQ(3, modelConverter.upgradeNodeY("3d", nodeY));
   EXPECT_EQ(nodeY, modelConverter.upgradeNodeY("UnknownMode", nodeY));

   modellingMode = "3d";
   nodeX = 185, nodeY = 125;
   EXPECT_EQ(nodeX, modelConverter.upgradeNodeX(modellingMode, nodeX));
   EXPECT_EQ(nodeY, modelConverter.upgradeNodeY(modellingMode, nodeY));
   //tests to verify mesh size are correctly manipulated if out of range values are found
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaX("1d", 100.0));
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaX("1d", 500.0));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("3d", 500.0));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("UnknownMode", 500.0));

   EXPECT_EQ(100.0, modelConverter.upgradeDeltaY("1d", 100.0));
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaY("1d", 500.0));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("3d", 500.0));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("UnknownMode", 500.0));
   //tests to verify whether the description field is correctly manipulated
   EXPECT_EQ("Migrated from BPA", modelConverter.upgradeDescription("")); 
   EXPECT_EQ("TestProject NVG: Migrated from BPA", modelConverter.upgradeDescription("TestProject NVG"));

}