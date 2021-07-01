#include <cmath>
#include <string.h>

#include "SimulationOutputConverters.h"

// Setter for projectFileName
//---------------------------------------------------------------------------------//
void converters::SimulationOutputConverters::setProjectFileName(const std::string& fileName)
{
    m_projectFileName = fileName;
}

// Getter for projectFileName
//---------------------------------------------------------------------------------//
std::string converters::SimulationOutputConverters::getProjectFileName() const
{
    return m_projectFileName;
}

// Returns the 'alternative' value if the 'preferred' value is equal to 'undefinedValue' else returns the 'preferred' value
//---------------------------------------------------------------------------------//
double converters::selectDefined(double undefinedValue, double preferred, double alternative)
{
    return (preferred != undefinedValue ? preferred : alternative);
}

// Rounding off a decimal number to certain decimal places
//---------------------------------------------------------------------------------//
double converters::roundoff(double number, int decimalPlaces)
{
	const double factor = std::pow(10.0, decimalPlaces);
	return std::round(number * factor) / factor;
}

// Test if the property is computable at basement
//---------------------------------------------------------------------------------//
bool converters::isBasementProperty(const std::string& property)
{
    // this list of properties is not exhaustive; out of the default properties listed in the spec file,
    // these are the properties which can be computed at the sediment+basement
    if (property == "BulkDensity" || property == "Velocity" || property == "Depth" ||
        property == "Temperature" || property == "LithoStaticPressure") return true;
    else return false;
}

// Fetch the list of properties provided in command line arguments in a vector
//---------------------------------------------------------------------------------//
void converters::fetchPropertyList(const std::string& properties, std::vector<std::string>& propertyList)
{
    std::string tempString;
    for (int i = 0; i < properties.length(); ++i)
    {
        if (properties[i] != ',')
            tempString.push_back(properties[i]);

        if ((properties[i] == ',') || (i == properties.length() - 1))
        {
            propertyList.push_back(tempString);
            tempString = "";
        }
    }
}

// Convert the endian-ness of a float
//---------------------------------------------------------------------------------//
float converters::correctEndian(const float x)
{
    float number = x;
    const char* tempArrayOfBytes = reinterpret_cast<char*>(&number);

    const int size = sizeof(float);
    char tempInvertedArrayOfBytes[size];
    for (int i = 0; i < size; ++i)
    {
        tempInvertedArrayOfBytes[i] = tempArrayOfBytes[size - 1 - i];
    }

    float numberEndianCorrected = *(reinterpret_cast<float*>(tempInvertedArrayOfBytes));
    return numberEndianCorrected;
}

// Splits the string into separate strings as per the separator provided
//---------------------------------------------------------------------------------//
bool converters::splitString(char* string, char separator, char*& firstPart, char*& secondPart, char*& thirdPart)
{
    firstPart = nullptr;
    secondPart = nullptr;
    thirdPart = nullptr;
    char* tail;
    if (!string || strlen(string) == 0) return false;

    /***/

    firstPart = string;
    tail = strchr(firstPart, separator);

    if (tail == nullptr) return false;

    if (tail == firstPart) firstPart = nullptr;

    *tail = '\0';
    ++tail;
    if (*tail == '\0')return false;

    /***/

    secondPart = tail;
    tail = strchr(secondPart, separator);

    if (tail == nullptr) return false;

    if (tail == secondPart) secondPart = nullptr;

    *tail = '\0';
    ++tail;
    if (*tail == '\0') return false;

    /***/

    thirdPart = tail;

    return true;
}
