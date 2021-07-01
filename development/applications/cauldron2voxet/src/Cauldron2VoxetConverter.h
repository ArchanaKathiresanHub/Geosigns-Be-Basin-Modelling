//
// Copyright (C) 2021-2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CAULDRON2VOXET_CONVERTER_H
#define CAULDRON2VOXET_CONVERTER_H

#include <vector>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#include <process.h>
#include "values_win.h"
#else
#include <unistd.h>
#include <values.h>
#endif // WIN_32

#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

// DataAccess API includes
#include "GridMap.h"
#include "Grid.h"
#include "Snapshot.h"
#include "Surface.h"
#include "Formation.h"
#include "Reservoir.h"
#include "LithoType.h"
#include "Property.h"
#include "PropertyValue.h"
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "SimulationDetails.h"

// GeoPhysics library
#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "SurfaceProperty.h"
#include "PrimarySurfaceProperty.h"
#include "FormationPropertyAtSurface.h"

#include "DepthInterpolator.h"
#include "LayerInterpolator.h"
#include "GridDescription.h"
#include "VoxetCalculator.h"
#include "VoxetPropertyGrid.h"
#include "array.h"

#include "CauldronProperty.h"
#include "VoxetProjectHandle.h"
#include "voxetschema.h"
#include "voxetschemafuncs.h"

#include "SimulationOutputConverters.h"

namespace cauldron_to_voxet
{
    struct VoxetAttributes
    {
        double originX = MAXDOUBLE;
        double originY = MAXDOUBLE;
        double originZ = MAXDOUBLE;

        double deltaX = MAXDOUBLE;
        double deltaY = MAXDOUBLE;
        double deltaZ = MAXDOUBLE;

        double countX = MAXDOUBLE;
        double countY = MAXDOUBLE;
        double countZ = MAXDOUBLE;
    };

    struct ArgumentList
    {
        char* argv0 = nullptr;

        bool useBasement = true;
        bool verbose = false;
        bool singlePropertyHeader = false;
        bool debug = false;

        std::string timeStamp = "";
        std::string properties = "";

        double snapshotTime = 0;

        std::string projectFileName;
        std::string voxetFileName;
        std::string createVoxetFileName;
        std::string outputFileName;

        std::vector<std::string> propertyList;

        cauldron_to_voxet::VoxetAttributes voxetDimensions;

        std::map<std::string, double> propertyNullValueReplaceLookup = std::map<std::string, double>();
    };

    struct cauldronProperty
    {
        std::string propertyName;
        std::string unit;
        double conversion;
        std::string outputPropertyName;
    };

    class Cauldron2VoxetConverter : public converters::SimulationOutputConverters
    {
    private:

        bool m_useBasement;
        bool m_verbose;
        bool m_singlePropertyHeader;
        bool m_debug;

        std::string m_voxetFileName;
        std::string m_createVoxetFileName;
        std::string m_outputFileName;
        std::string m_timeStamp;
        std::string m_properties;
        std::vector<std::string> m_propertyList;

        VoxetAttributes m_voxetDimensions;

        std::map<std::string, double> m_propertyNullValueReplaceLookup;

        char* m_argv0;
        double m_snapshotTime;

        std::vector<cauldronProperty> m_cauldronPropertiesForVoxet;

    public:

        /// @brief Parameterized constructor taking in a structure object containing all the command line arguments as an argument
        explicit Cauldron2VoxetConverter(const cauldron_to_voxet::ArgumentList& arguments);

        /// @brief Displays all the command line options available to users for the cauldron2voxet application
        /// @details Called automatically if user passed invalid command line arguments
        static void showUsage(const char* message, const char* argv0);

        /// @brief Compute function to generate the voxets
        void compute() override;

    private:

        /// @brief Write the values to the specified file.
        void write(const std::string& name, 
            const VoxetPropertyGrid& values) const;

        /// @brief Correct the endian-ness of the array. 
        /// @details The voxet format requires that the binary data is written in big-endian format. After this call the numbers will be un-usable in the code.
        void correctEndian(VoxetPropertyGrid& values) const;

        /// @brief Write the ascii voxet header (vo-file)
        void writeVOheader(ofstream& file,
            const GridDescription& gridDescription,
            const std::string& outputFileName) const;

        /// @brief Write the property information into the voxet header (vo-file)
        void writeVOproperty(ofstream& file,
            const int& propertyCount,
            const CauldronProperty* cauldronProperty,
            const string& propertyFileName,
            const float& nullValue) const;

        /// @brief Write the tail of the  ascii voxet header (vo-file)
        void writeVOtail(ofstream& file) const;

        /// @brief Print to stdout a default voxet file based on the cauldron project file that has been input.
        void createVoxetProjectFile(const DataAccess::Interface::ProjectHandle& cauldronProject,
            DerivedProperties::DerivedPropertyManager& propertyManager,
            ostream& outputStream, const DataAccess::Interface::Snapshot* snapshot, 
            const std::vector<std::string>& propertyList, 
            const VoxetAttributes& voxetDimensions) const;
    };

    // Functions to read the command line arguments

    /// @brief Check if any flag to display help is provided at the command line
    void readHelpFlagsFromCmdLine(std::vector<std::string>& argList);

    /// @brief Read the string next to the 'parameterToRead' in the 'outString'
    void readSingleStringFromCmdLine(std::vector<std::string>& argList, const std::string& parameterToRead, std::string& outString);

    /// @brief Read the Voxet grid parameter 'parameterToRead' into the floating point numbers 'voxetParamX', 'voxetParamY','voxetParamZ'
    void readVoxetAttribsFromCmdLine(std::vector<std::string>& argList, const std::string& parameterToRead, double& voxetParamX, double& voxetParamY, double& voxetParamZ);

    /// @brief Read the 'nullvaluereplace' and the properties provided
    void readNullValueReplaceFromCmdLine(std::vector<std::string>& argList, ArgumentList& arguments);

    /// @brief Returns true if 'parameterToRead' is provided at the command line
    bool readBoolFlagsFromCmdLine(std::vector<std::string>& argList, const std::string& parameterToRead);
}
#endif
