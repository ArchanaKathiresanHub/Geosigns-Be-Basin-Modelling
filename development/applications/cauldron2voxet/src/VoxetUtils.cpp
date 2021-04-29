#include "VoxetUtils.h"

void VoxetUtils::write(const std::string& name,
    const VoxetPropertyGrid& values)
{

    FILE* file;

    file = fopen(name.c_str(), "w");

    if (file == nullptr)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " cannot open file: " << name << " for writing";
        return;
    }

    fwrite(values.getOneDData(), sizeof(float), values.getGridDescription().getVoxetNodeCount(), file);
    fclose(file);
}

//------------------------------------------------------------//
float VoxetUtils::correctEndian(const float x)
{
    float number = x;
    char* tempArrayOfBytes = reinterpret_cast<char*>(&number);

    const int size = sizeof(float);
    char tempInvertedArrayOfBytes[size];
    for (int i = 0; i < size; ++i)
    {
        tempInvertedArrayOfBytes[i] = tempArrayOfBytes[size-1-i];
    }

    float numberEndianCorrected = *(reinterpret_cast<float*>(tempInvertedArrayOfBytes));
    return numberEndianCorrected;
}

//------------------------------------------------------------//
void VoxetUtils::correctEndian(VoxetPropertyGrid& values)
{
    const int nx = values.getGridDescription().getVoxetNodeCount(0);
    const int ny = values.getGridDescription().getVoxetNodeCount(1);
    const int nz = values.getGridDescription().getVoxetNodeCount(2);

    int i;
    int j;
    int k;

    for (k = 0; k < nz; ++k)
    {
        for (j = 0; j < ny; ++j)
        {
            for (i = 0; i < nx; ++i)
            {
                values(i, j, k) = VoxetUtils::correctEndian(values(i, j, k));
            }
        }
    }
}

//------------------------------------------------------------//
bool VoxetUtils::splitString(char* string, char separator, char*& firstPart, char*& secondPart, char*& thirdPart)
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

//------------------------------------------------------------//
double VoxetUtils::selectDefined(double undefinedValue, double preferred, double alternative)
{
    return (preferred != undefinedValue ? preferred : alternative);
}

//------------------------------------------------------------//
void VoxetUtils::writeVOheader(ofstream& file,
    const GridDescription& gridDescription,
    const string& outputFileName)
{
    file << "GOCAD Voxet 1.0" << endl;
    file << "HEADER" << endl;
    file << "{" << endl;
    file << "name:" << outputFileName << endl;
    file << "}" << endl;

    file << "GOCAD_ORIGINAL_COORDINATE_SYSTEM" << endl;
    file << "NAME Default" << endl;
    file << "AXIS_NAME \"X\" \"Y\" \"Z\" " << endl;
    file << "AXIS_UNIT \"m\" \"m\" \"m\" " << endl;
    file << "ZPOSITIVE Depth" << endl;
    file << "END_ORIGINAL_COORDINATE_SYSTEM" << endl;

    file << "AXIS_O "
        << gridDescription.getVoxetGridOrigin(0) << "  "
        << gridDescription.getVoxetGridOrigin(1) << "  "
        << gridDescription.getVoxetGridOrigin(2) << "  " << endl;


    file << "AXIS_U " << gridDescription.getVoxetGridMaximum(0) - gridDescription.getVoxetGridOrigin(0) << "  " << " 0.0  0.0 " << endl;
    file << "AXIS_V  0.0 " << gridDescription.getVoxetGridMaximum(1) - gridDescription.getVoxetGridOrigin(1) << "  " << " 0.0  " << endl;
    file << "AXIS_W 0.0 0.0  " << gridDescription.getVoxetGridMaximum(2) - gridDescription.getVoxetGridOrigin(2) << "  " << endl;
    file << "AXIS_MIN 0.0 0.0 0.0 " << endl;
    file << "AXIS_MAX 1  1  1" << endl;

    file << "AXIS_N "
        << gridDescription.getVoxetNodeCount(0) << "  "
        << gridDescription.getVoxetNodeCount(1) << "  "
        << gridDescription.getVoxetNodeCount(2) << "  " << endl;

    file << "AXIS_NAME \"X\" \"Y\" \"Z\" " << endl;
    file << "AXIS_UNIT \"m\" \"m\" \"m\" " << endl;
    file << "AXIS_TYPE even even even" << endl;

    file << endl;
}

//------------------------------------------------------------//
void VoxetUtils::writeVOproperty(ofstream& file,
    const int& propertyCount,
    const CauldronProperty* cauldronProperty,
    const string& propertyFileName,
    const float& nullValue) {
    file << "PROPERTY " << propertyCount << "  \"" << cauldronProperty->getVoxetName() << '"' << endl;
    file << "PROPERTY_KIND " << propertyCount << "  \"" << cauldronProperty->getVoxetName() << '"' << endl;
    file << "PROPERTY_CLASS " << propertyCount << " \"" << cauldronProperty->getVoxetName() << '"' << endl;
    file << "PROPERTY_CLASS_HEADER " << propertyCount << " \"" << cauldronProperty->getVoxetName() << "\" {" << endl;
    file << "name:" << cauldronProperty->getVoxetName() << endl << "}" << endl;

    if (cauldronProperty->getCauldronName() == "Depth")
    {
        file << "PROPERTY_SUBCLASS " << propertyCount << " " << "LINEARFUNCTION Float -1 0" << endl;
    }
    else
    {
        file << "PROPERTY_SUBCLASS " << propertyCount << " " << "QUANTITY Float" << endl;
    }

    file << "PROP_ORIGINAL_UNIT " << propertyCount << " " << cauldronProperty->getUnits() << endl;
    file << "PROP_UNIT " << propertyCount << " " << cauldronProperty->getUnits() << endl;
    file << "PROP_ESIZE " << propertyCount << " " << sizeof(float) << endl;
    file << "PROP_ETYPE " << propertyCount << " IEEE " << endl;

    if (cauldronProperty->getCauldronName() == "Depth")
    {
        file << "PROP_NO_DATA_VALUE " << propertyCount << " " << -1*nullValue << endl;
    }
    else
    {
        file << "PROP_NO_DATA_VALUE " << propertyCount << " " << nullValue << endl;
    }
    file << "PROP_FILE " << propertyCount << " " << propertyFileName << endl;
    file << endl;
}

//------------------------------------------------------------//
void VoxetUtils::writeVOtail(ofstream& file)
{
    file << "END" << endl;
}

//------------------------------------------------------------//
bool VoxetUtils::isBasementProperty(const std::string& property)
{
    // this list of properties is not exhaustive; out of the default properties listed in the spec file,
    // these are the properties which can be computed at the sediment+basement
    if (property == "BulkDensity" || property == "Velocity" || property == "Depth" ||
        property == "Temperature" || property == "LithoStaticPressure") return true;
    else return false;
}

//------------------------------------------------------------//
double VoxetUtils::roundoff(double number, int decimalPlaces)
{
    const double factor = std::pow(10.0,decimalPlaces);
    return std::round(number*factor)/factor;
}
