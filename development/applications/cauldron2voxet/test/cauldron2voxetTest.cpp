#include <gtest/gtest.h>

#include "../src/VoxetUtils.h"

TEST(cauldron2voxet, voxetUtils)
{
    EXPECT_EQ(true, VoxetUtils::isBasementProperty("BulkDensity"));
    EXPECT_EQ(true, VoxetUtils::isBasementProperty("Velocity"));
    EXPECT_EQ(true, VoxetUtils::isBasementProperty("Depth"));
    EXPECT_EQ(true, VoxetUtils::isBasementProperty("Temperature"));
    EXPECT_EQ(true, VoxetUtils::isBasementProperty("LithoStaticPressure"));

    EXPECT_EQ(false, VoxetUtils::isBasementProperty("Pressure"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("OverPressure"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("HydroStaticPressure"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("Vr"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("Ves"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("MaxVes"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("Porosity"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("Permeability"));
    EXPECT_EQ(false, VoxetUtils::isBasementProperty("TwoWayTime"));

    EXPECT_EQ(false, VoxetUtils::isBasementProperty("abc"));

    EXPECT_EQ(1.193, VoxetUtils::roundoff(1.19334,3));
    EXPECT_EQ(1.194, VoxetUtils::roundoff(1.19354,3));
    EXPECT_EQ(1.194, VoxetUtils::roundoff(1.19374,3));

    EXPECT_EQ(123.123, VoxetUtils::selectDefined(99999,99999,123.123));
    EXPECT_EQ(456.456, VoxetUtils::selectDefined(99999,456.456,123.123));

    EXPECT_EQ(VoxetUtils::roundoff(5.74869,5), VoxetUtils::roundoff(VoxetUtils::correctEndian(5.0f)*1e41,5));
    EXPECT_EQ(VoxetUtils::roundoff(6.32266,5), VoxetUtils::roundoff(VoxetUtils::correctEndian(5.5f)*1e41,5));
    EXPECT_EQ(VoxetUtils::roundoff(6.89663,5), VoxetUtils::roundoff(VoxetUtils::correctEndian(6.0f)*1e41,5));
    EXPECT_EQ(VoxetUtils::roundoff(7.4706,4), VoxetUtils::roundoff(VoxetUtils::correctEndian(6.5f)*1e41,4));

    string properties = "Depth,Temperature,OverPressure,TwoWayTime,HydroStaticPressure,MaxVes,Porosity";
    std::vector<string> propertyList;
    VoxetUtils::fetchPropertyList(properties, propertyList);
    EXPECT_EQ("Depth",propertyList[0]);
    EXPECT_EQ("Temperature",propertyList[1]);
    EXPECT_EQ("OverPressure",propertyList[2]);
    EXPECT_EQ("TwoWayTime",propertyList[3]);
    EXPECT_EQ("HydroStaticPressure",propertyList[4]);
    EXPECT_EQ("MaxVes",propertyList[5]);
    EXPECT_EQ("Porosity",propertyList[6]);
}

