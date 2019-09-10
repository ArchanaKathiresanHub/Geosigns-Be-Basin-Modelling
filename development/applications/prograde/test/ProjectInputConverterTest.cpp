#include <gtest/gtest.h>

#include "ProjectIoModelConverter.h"


//cmbAPI
#include "ProjectDataManager.h"


using namespace mbapi;

TEST(ProjectIoModelConverter, upgrade)
{
   Prograde::ProjectIoModelConverter modelConverter;
   std::string modellingMode="1d";
   EXPECT_EQ("3d", modelConverter.upgradeModellingMode(modellingMode));
   EXPECT_EQ("3d", modelConverter.upgradeModellingMode("3d"));
   EXPECT_EQ("UnknownMode", modelConverter.upgradeModellingMode("UnknownMode"));

   //tests to verify whether the number of nodes are correctly manipulated if out of range values are found for the same
   int nodeX = 1, nodeY = 1, OriginalWindowMax=0, NewWindowMax;
   EXPECT_EQ(2, modelConverter.upgradeNodeX("1d", nodeX, OriginalWindowMax, NewWindowMax));//when original nodeX is less than 2 for 1d cases 
   EXPECT_EQ(1, NewWindowMax);
   EXPECT_EQ(2, modelConverter.upgradeNodeX("1d", 5, 4, NewWindowMax));//when original nodeX is more than 2 for 1d cases
   EXPECT_EQ(1, NewWindowMax);
   EXPECT_EQ(2, modelConverter.upgradeNodeX("1d", 2, 1, NewWindowMax));//when original nodeX is equal to 2 for 1d cases
   EXPECT_EQ(1, NewWindowMax);
   EXPECT_EQ(3, modelConverter.upgradeNodeX("3d", 1, OriginalWindowMax, NewWindowMax));//when original nodeX is less than 2 for 3d cases
   EXPECT_EQ(2, NewWindowMax);
   EXPECT_EQ(3, modelConverter.upgradeNodeX("3d", 2, 1, NewWindowMax));//when original nodeX is less than 2 for 3d cases
   EXPECT_EQ(2, NewWindowMax);
   EXPECT_EQ(185, modelConverter.upgradeNodeX("3d", 185, 184, NewWindowMax));//when original nodeX is within the allowed limit for 3d cases
   EXPECT_EQ(184, NewWindowMax);
   EXPECT_EQ(nodeX, modelConverter.upgradeNodeX("UnknownMode", nodeX, OriginalWindowMax, NewWindowMax));//when unknown modelling mode is found
   EXPECT_EQ(OriginalWindowMax, NewWindowMax);
   //tests for nodeY counts
   EXPECT_EQ(2, modelConverter.upgradeNodeY("1d", nodeY, OriginalWindowMax, NewWindowMax));//when original nodeY is less than 2 for 1d cases 
   EXPECT_EQ(1, NewWindowMax);
   EXPECT_EQ(2, modelConverter.upgradeNodeY("1d", 5, 4, NewWindowMax));//when original nodeY is more than 2 for 1d cases
   EXPECT_EQ(1, NewWindowMax);
   EXPECT_EQ(2, modelConverter.upgradeNodeY("1d", 2, 1, NewWindowMax));//when original nodeY is equal to 2 for 1d cases
   EXPECT_EQ(1, NewWindowMax);
   EXPECT_EQ(3, modelConverter.upgradeNodeY("3d", 1, OriginalWindowMax, NewWindowMax));//when original nodeY is less than 2 for 3d cases
   EXPECT_EQ(2, NewWindowMax);
   EXPECT_EQ(3, modelConverter.upgradeNodeY("3d", 2, 1, NewWindowMax));//when original nodeY is less than 2 for 3d cases
   EXPECT_EQ(2, NewWindowMax);
   EXPECT_EQ(125, modelConverter.upgradeNodeY("3d", 125, 184, NewWindowMax));//when original nodeY is within the allowed limit for 3d cases
   EXPECT_EQ(184, NewWindowMax);
   EXPECT_EQ(nodeX, modelConverter.upgradeNodeY("UnknownMode", nodeY, OriginalWindowMax, NewWindowMax));//when unknown modelling mode is found
   EXPECT_EQ(OriginalWindowMax, NewWindowMax);

   //tests to verify whether grid spacings are manipulated correctly if out of range values are found for node counts 
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaX("1d", 100.0));//default value is found...nothing to do
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaX("1d", 500.0));//default value is not found...upgraded to default value
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("3d", 500.0));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaX("UnknownMode", 500.0));

   EXPECT_EQ(100.0, modelConverter.upgradeDeltaY("1d", 100.0));//default value is found...nothing to do
   EXPECT_EQ(100.0, modelConverter.upgradeDeltaY("1d", 500.0));//default value is not found...upgraded to default value
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("3d", 500.0));
   EXPECT_EQ(500.0, modelConverter.upgradeDeltaY("UnknownMode", 500.0));

   //tests to verify whether the description field is correctly manipulated
   EXPECT_EQ("Migrated from BPA", modelConverter.upgradeDescription("3d","")); 
   EXPECT_EQ("Migrated from BPA", modelConverter.upgradeDescription("1d", ""));
   EXPECT_EQ("Migrated from BPA (Please note that the original modelling mode was 1Dand3D)", modelConverter.upgradeDescription("both", ""));
   EXPECT_EQ("Migrated from BPA", modelConverter.upgradeDescription("UnknownModellingMode", ""));
   EXPECT_EQ("TestProject NVG: Migrated from BPA", modelConverter.upgradeDescription("3d","TestProject NVG"));
   EXPECT_EQ("TestProject NVG: Migrated from BPA", modelConverter.upgradeDescription("1d", "TestProject NVG"));
   EXPECT_EQ("TestProject NVG: Migrated from BPA (Please note that the original modelling mode was 1Dand3D)", modelConverter.upgradeDescription("both", "TestProject NVG"));
   EXPECT_EQ("TestProject NVG: Migrated from BPA", modelConverter.upgradeDescription("UnknownModellingMode", "TestProject NVG"));
}