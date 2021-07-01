//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Cauldron2VoxetConverter.h"

int main (int argc, char ** argv)
{
    //[1] Read the command line arguments

    /// Object to store the command line arguments
    cauldron_to_voxet::ArgumentList arguments;

    if ((arguments.argv0 = strrchr(argv[0], '/')) != nullptr)
    {
        ++arguments.argv0;
        argv[0] = arguments.argv0;
    }
    else
    {
        arguments.argv0 = argv[0];
    }

    std::vector<std::string> argList;
    for (int arg = 0; arg < argc; arg++)
    {
        argList.emplace_back(argv[arg]);
    }

    if (argList.size() == 1)
    {
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(" Standard usage.", (argList.at(0)).c_str());
        exit(-1);
    }

    cauldron_to_voxet::readHelpFlagsFromCmdLine(argList);

    std::string outString;

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-project", outString);
    arguments.projectFileName = outString;

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-spec", outString);
    arguments.voxetFileName = outString;

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-snapshot", outString);
    arguments.snapshotTime = atof(outString.c_str());

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-output", outString);
    arguments.outputFileName = outString;

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-time", outString);
    // "_" is appended here to avoid a condition on existence of timeStamp at later part of the code
    if (outString.compare(""))
    {
        arguments.timeStamp = "_" + outString;
    }
    
    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-create-spec", outString);
    arguments.createVoxetFileName = outString;

    cauldron_to_voxet::readSingleStringFromCmdLine(argList, "-properties", outString);
    arguments.properties = outString;
    converters::fetchPropertyList(arguments.properties, arguments.propertyList);

    cauldron_to_voxet::readVoxetAttribsFromCmdLine(argList, "-origin", arguments.voxetDimensions.originX, arguments.voxetDimensions.originY, arguments.voxetDimensions.originZ);
    cauldron_to_voxet::readVoxetAttribsFromCmdLine(argList, "-delta", arguments.voxetDimensions.deltaX, arguments.voxetDimensions.deltaY, arguments.voxetDimensions.deltaZ);
    cauldron_to_voxet::readVoxetAttribsFromCmdLine(argList, "-count", arguments.voxetDimensions.countX, arguments.voxetDimensions.countY, arguments.voxetDimensions.countZ);

    cauldron_to_voxet::readNullValueReplaceFromCmdLine(argList, arguments);

    arguments.debug = cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-debug");
    arguments.useBasement = !cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-nobasement");
    arguments.verbose = cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-verbose");
    arguments.singlePropertyHeader = cauldron_to_voxet::readBoolFlagsFromCmdLine(argList, "-singlepropertyheader");

    if (argList.size() > 1)
    {
        std::string message = "Illegal Argument: " + argList.at(1);
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(message.c_str(), (argList.at(0)).c_str());
        exit(-1);
    }

    if (arguments.projectFileName == "")
    {
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage("No project file specified", arguments.argv0);
        exit(-1);
    }

    if (arguments.outputFileName == "")
    {
        size_t dotPos = arguments.projectFileName.find(".project");
        arguments.outputFileName = arguments.projectFileName.substr(0, dotPos);
    }
    
    //[2] Initializing a Cauldron2VoxetConverter instance with the instance of ArgumentList
    cauldron_to_voxet::Cauldron2VoxetConverter voxetConverter(arguments);

    //[3] Computing and generating the property voxets
    voxetConverter.compute();
    
    return 0;
}
