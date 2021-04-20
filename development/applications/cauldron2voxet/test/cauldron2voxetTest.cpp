#include <gtest/gtest.h>

#include "../src/VoxetUtils.h"

TEST(cauldron2voxet, testBasementProperties)
{
    EXPECT_EQ(true,isBasementProperty("BulkDensity"));
    EXPECT_EQ(true,isBasementProperty("Velocity"));
    EXPECT_EQ(true,isBasementProperty("Depth"));
    EXPECT_EQ(true,isBasementProperty("Temperature"));
    EXPECT_EQ(true,isBasementProperty("LithoStaticPressure"));

    EXPECT_EQ(false,isBasementProperty("Pressure"));
    EXPECT_EQ(false,isBasementProperty("OverPressure"));
    EXPECT_EQ(false,isBasementProperty("HydroStaticPressure"));
    EXPECT_EQ(false,isBasementProperty("Vr"));
    EXPECT_EQ(false,isBasementProperty("Ves"));
    EXPECT_EQ(false,isBasementProperty("MaxVes"));
    EXPECT_EQ(false,isBasementProperty("Porosity"));
    EXPECT_EQ(false,isBasementProperty("Permeability"));
    EXPECT_EQ(false,isBasementProperty("TwoWayTime"));

    EXPECT_EQ(false,isBasementProperty("abc"));
}
