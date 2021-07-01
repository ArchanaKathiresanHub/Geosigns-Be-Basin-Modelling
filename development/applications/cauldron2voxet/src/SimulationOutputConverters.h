//
// Copyright (C) 2021-2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef SIMULATION_OUTPUT_CONVERTERS_H
#define SIMULATION_OUTPUT_CONVERTERS_H

#include <string>
#include <vector>

/// <summary>
///  Simulation Output Converters class
///  Base class to all other converters -
///  1.cauldron2voxet 2.cauldron2eclipse 3.fastproperties
/// </summary>
namespace converters
{
    /// @class
    /// @brief Abstract class as a parent class for the output converters 
    class SimulationOutputConverters
    {
    private:

        /// @brief Name of the project3d file of the scenario
        std::string m_projectFileName;

    protected:

        /// @brief Setter for m_projectFileName
        void setProjectFileName(const std::string& fileName);

        /// @brief Getter for m_projectFileName
        std::string getProjectFileName() const;

    public:

        /// @brief Constructor
        SimulationOutputConverters() = default;

        /// @brief Computes the property data needed and generates the data files in the desired format
        virtual void compute() = 0;

        /// @brief Destructor
        virtual ~SimulationOutputConverters() = default;
    };

    // Utility functions for converters

    /// @brief Returns the 'alternative' value if the 'preferred' value is equal to 'undefinedValue' else returns the 'preferred' value
    double selectDefined(double undefinedValue, double preferred, double alternative);

    /// @brief Roundoff a decimal number to certain decimal places
    double roundoff(double number, int decimalPlaces);

    /// @brief Test if the property is computable at basement
    bool isBasementProperty(const std::string& property);

    /// @brief Fetch the list of properties provided in command line arguments in a vector
    void fetchPropertyList(const std::string& properties, std::vector<std::string>& propertyList);

    /// @brief Convert the endian-ness of a float
    float correctEndian(const float x);

    /// @brief Splits the string into separate strings as per the separator provided
    bool splitString(char* string, char separator, char*& firstPart, char*& secondPart, char*& thirdPart);
}

#endif
