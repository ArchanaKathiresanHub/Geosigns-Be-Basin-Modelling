

#include <gtest/gtest.h>

#include "StratigraphyModelConverter.h"
using namespace mbapi;

TEST(StratigraphyModelConverter, upgrade)
{
   Prograde::StratigraphyModelConverter modelConverter;
   
   EXPECT_EQ("Abc____123", modelConverter.upgradeName("Abc@%$#123"));
   EXPECT_EQ("FluidName", modelConverter.upgradeName("FluidName"));
   EXPECT_EQ("Fluid Name", modelConverter.upgradeName("Fluid Name"));
   EXPECT_EQ("Fluid_Name", modelConverter.upgradeName("Fluid_Name"));
   EXPECT_EQ(6380000, modelConverter.upgradeDepthThickness(70000000));
   EXPECT_EQ(-6380000, modelConverter.upgradeDepthThickness(-700000000));
   EXPECT_EQ(850000, modelConverter.upgradeDepthThickness(850000));
   EXPECT_EQ(-850000, modelConverter.upgradeDepthThickness(-850000));
   EXPECT_EQ(-9999, modelConverter.upgradeLayeringIndex("Homogeneous", 1));
   EXPECT_EQ(2, modelConverter.upgradeLayeringIndex("AnyThing", 2));
   EXPECT_EQ(0, modelConverter.checkChemicalCompaction("Hydrostatic", 1));
   EXPECT_EQ(5, modelConverter.checkChemicalCompaction("anything", 5));
   EXPECT_EQ(0, modelConverter.checkChemicalCompaction("Hydrostatic", 0));
   EXPECT_EQ(1, modelConverter.upgradeChemicalCompaction(1, 0));
   EXPECT_EQ(0, modelConverter.upgradeChemicalCompaction(0, 1));
}