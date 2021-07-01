#include <gtest/gtest.h>

#include "../src/SimulationOutputConverters.h"
#include "../src/Cauldron2VoxetConverter.h"

TEST(cauldron2voxet, voxetsUnitTests)
{

    EXPECT_EQ(true, converters::isBasementProperty("BulkDensity"));
    EXPECT_EQ(true, converters::isBasementProperty("Velocity"));
    EXPECT_EQ(true, converters::isBasementProperty("Depth"));
    EXPECT_EQ(true, converters::isBasementProperty("Temperature"));
    EXPECT_EQ(true, converters::isBasementProperty("LithoStaticPressure"));

    EXPECT_EQ(false, converters::isBasementProperty("Pressure"));
    EXPECT_EQ(false, converters::isBasementProperty("OverPressure"));
    EXPECT_EQ(false, converters::isBasementProperty("HydroStaticPressure"));
    EXPECT_EQ(false, converters::isBasementProperty("Vr"));
    EXPECT_EQ(false, converters::isBasementProperty("Ves"));
    EXPECT_EQ(false, converters::isBasementProperty("MaxVes"));
    EXPECT_EQ(false, converters::isBasementProperty("Porosity"));
    EXPECT_EQ(false, converters::isBasementProperty("Permeability"));
    EXPECT_EQ(false, converters::isBasementProperty("TwoWayTime"));

    EXPECT_EQ(false, converters::isBasementProperty("abc"));

    EXPECT_EQ(1.193, converters::roundoff(1.19334, 3));
    EXPECT_EQ(1.194, converters::roundoff(1.19354, 3));
    EXPECT_EQ(1.194, converters::roundoff(1.19374, 3));

    EXPECT_EQ(123.123, converters::selectDefined(99999, 99999, 123.123));
    EXPECT_EQ(456.456, converters::selectDefined(99999, 456.456, 123.123));

    EXPECT_EQ(converters::roundoff(5.74869, 5), converters::roundoff(converters::correctEndian(5.0f) * 1e41, 5));
    EXPECT_EQ(converters::roundoff(6.32266, 5), converters::roundoff(converters::correctEndian(5.5f) * 1e41, 5));
    EXPECT_EQ(converters::roundoff(6.89663, 5), converters::roundoff(converters::correctEndian(6.0f) * 1e41, 5));
    EXPECT_EQ(converters::roundoff(7.4706, 4), converters::roundoff(converters::correctEndian(6.5f) * 1e41, 4));

    std::string properties = "Depth,Temperature,OverPressure,TwoWayTime,HydroStaticPressure,MaxVes,Porosity";
    std::vector<std::string> propertyList;
    converters::fetchPropertyList(properties, propertyList);
    EXPECT_EQ("Depth", propertyList[0]);
    EXPECT_EQ("Temperature", propertyList[1]);
    EXPECT_EQ("OverPressure", propertyList[2]);
    EXPECT_EQ("TwoWayTime", propertyList[3]);
    EXPECT_EQ("HydroStaticPressure", propertyList[4]);
    EXPECT_EQ("MaxVes", propertyList[5]);
    EXPECT_EQ("Porosity", propertyList[6]);

    std::string testString = "1234,567,890";
    char* c_testString = &testString[0];
    char* firstPart;
    char* secondPart;
    char* thirdPart;
    bool flag = converters::splitString(c_testString,',' ,firstPart, secondPart, thirdPart);

    EXPECT_EQ(0, strcmp("1234", firstPart));
    EXPECT_EQ(0, strcmp("567", secondPart));
    EXPECT_EQ(0, strcmp("890", thirdPart));
    EXPECT_EQ(true, flag);

    std::vector<std::string> argList;
    argList.reserve(28);
    argList.emplace_back("cauldron2voxet");
    argList.emplace_back("-project");
    argList.emplace_back("Project.project3d");
    argList.emplace_back("-snapshot");
    argList.emplace_back("0");
    argList.emplace_back("-origin");
    argList.emplace_back("526000,6730000,3.823");
    argList.emplace_back("-delta");
    argList.emplace_back("1000,1000,100");
    argList.emplace_back("-count");
    argList.emplace_back("53,54,77");
    argList.emplace_back("-output");
    argList.emplace_back("Ottoland-Scenario");
    argList.emplace_back("-time");
    argList.emplace_back("20210621-070100");
    argList.emplace_back("-nullvaluereplace");
    argList.emplace_back("BulkDensity,1030");
    argList.emplace_back("Velocity,1500");
    argList.emplace_back("-verbose");
    argList.emplace_back("-debug");
    argList.emplace_back("-nobasement");
    argList.emplace_back("-singlepropertyheader");
    argList.emplace_back("-properties");
    argList.emplace_back("Depth,BulkDensity,OverPressure,TwoWayTime,HydroStaticPressure,Velocity,Porosity");
    argList.emplace_back("-create-spec");
    argList.emplace_back("specGen");
    argList.emplace_back("-spec");
    argList.emplace_back("file");

    std::string outString;
    cauldron_to_voxet::readSingleStringFromCmdLine(argList,"-project", outString);
    EXPECT_EQ(0, outString.compare("Project.project3d"));

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-snapshot", outString);
    EXPECT_EQ(0, outString.compare("0"));

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-output", outString);
    EXPECT_EQ(0, outString.compare("Ottoland-Scenario"));

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-time", outString);
    EXPECT_EQ(0, outString.compare("20210621-070100"));

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-create-spec", outString);
    EXPECT_EQ(0, outString.compare("specGen"));

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-spec", outString);
    EXPECT_EQ(0, outString.compare("file"));

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-properties", outString);
    EXPECT_EQ(0, outString.compare("Depth,BulkDensity,OverPressure,TwoWayTime,HydroStaticPressure,Velocity,Porosity"));

    double numX = 0;
    double numY = 0;
    double numZ = 0;

    cauldron_to_voxet::readVoxetAttribsFromCmdLine(argList, "-origin", numX, numY, numZ);
    EXPECT_EQ(526000, numX);
    EXPECT_EQ(6730000, numY);
    EXPECT_EQ(3.823, numZ);
    
    cauldron_to_voxet::readVoxetAttribsFromCmdLine(argList, "-delta", numX, numY, numZ);
    EXPECT_EQ(1000, numX);
    EXPECT_EQ(1000, numY);
    EXPECT_EQ(100, numZ);

    cauldron_to_voxet::readVoxetAttribsFromCmdLine(argList, "-count", numX, numY, numZ);
    EXPECT_EQ(53, numX);
    EXPECT_EQ(54, numY);
    EXPECT_EQ(77, numZ);

    EXPECT_EQ(true, cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-debug"));
    EXPECT_EQ(true, cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-nobasement"));
    EXPECT_EQ(true, cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-verbose"));
    EXPECT_EQ(true, cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-singlepropertyheader"));
    
}


