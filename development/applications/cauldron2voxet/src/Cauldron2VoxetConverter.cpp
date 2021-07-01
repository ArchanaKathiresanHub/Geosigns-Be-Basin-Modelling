//
// Copyright (C) 2021-2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include <iostream>
#include <algorithm>

#include "NumericFunctions.h"
#include "Cauldron2VoxetConverter.h"

// Parameterized Constructor - initializing the object from another ArgumentList structure object
// ------------------------------------------------------------------------//
cauldron_to_voxet::Cauldron2VoxetConverter::Cauldron2VoxetConverter(const cauldron_to_voxet::ArgumentList& arguments)
    :m_useBasement(arguments.useBasement), m_verbose(arguments.verbose), m_singlePropertyHeader (arguments.singlePropertyHeader), 
    m_debug(arguments.debug), m_voxetFileName (arguments.voxetFileName), m_createVoxetFileName(arguments.createVoxetFileName), 
    m_outputFileName (arguments.outputFileName), m_timeStamp(arguments.timeStamp), m_properties(arguments.properties), 
    m_propertyList(arguments.propertyList), m_voxetDimensions(arguments.voxetDimensions), m_propertyNullValueReplaceLookup(arguments.propertyNullValueReplaceLookup), 
    m_argv0(arguments.argv0), m_snapshotTime(arguments.snapshotTime)
{
    
    // Initializing the m_projectFileName, the private member from SimulationOutputConverters (base class)
    setProjectFileName(arguments.projectFileName);

    // List of properties available for conversion to voxet
    m_cauldronPropertiesForVoxet.reserve(14);
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Depth", "m", 1, "Depth " }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Pressure", "MPa", 1, "Pressure" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "OverPressure", "MPa", 1, "Pressure: Overpressure" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "HydroStaticPressure", "MPa", 1, "Pressure: Hydrostatic" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "LithoStaticPressure", "MPa", 1, "Pressure: Lithostatic" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Temperature", "degC", 1, "Temperature" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Vr", "percent", 1, "Vr" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Ves", "Pa", 1, "Ves" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "MaxVes", "Pa", 1, "Max Ves" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Porosity", "percent", 1, "Porosity" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Permeability", "mD", 1, "Permeability" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "BulkDensity", "kg/m^3", 1, "Bulk Density" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "Velocity", "m/s", 1, "Velocity" }));
    m_cauldronPropertiesForVoxet.emplace_back(cauldronProperty({ "TwoWayTime", "ms", 1, "Two Way Time" }));

}

// Display the help message 
// ------------------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(const char* message, const char* argv0)
{
    std::cerr << std::endl;
    if (message)
    {
        std::cerr << argv0 << ": " << message << std::endl;
    }

    std::cerr << "Usage (Options may be abbreviated): " << std::endl
        << argv0 << "    -project <cauldron-project-file>" << std::endl
        << "                  [-spec <spec-file>]" << std::endl
        << "                  [-snapshot <age>]" << std::endl
        << "                  [-origin <originX>,<originY>,<originZ>]" << std::endl
        << "                  [-delta <deltaX>,<deltaY>,<deltaZ>]" << std::endl
        << "                  [-count <countX>,<countY>,<countZ>]" << std::endl
        << "                  [-output <output-file-name>]" << std::endl
        << "                  [-time <time-stamp>]" << std::endl
        << "                  [-create-spec <spec-file>]" << std::endl
        << "                  [-nullvaluereplace <PropertyName,Value> [<PropertyName,Value>] [...]]" << std::endl
        << "                  [-nobasement]" << std::endl
        << "                  [-singlepropertyheader]" << std::endl
        << "                  [-properties <PropertyName1,PropertyName2,...>]" << std::endl
        << "                  [-verbose]" << std::endl
        << "                  [-help]" << std::endl
        << "                  [-?]" << std::endl
        << "                  [-usage]" << std::endl
        << std::endl
        << "    -project              The cauldron project file." << std::endl
        << "    -spec                 Use the specified spec file. Use a standard spec file if missing." << std::endl
        << "    -snapshot             Use the specified snapshot age. Not valid in conjunction with '-spec'," << std::endl
        << "    -origin               Use the specified coordinates as the origin of the sample cube" << std::endl
        << "    -delta                Use the specified values as the sampling distance in the x, y and z direction" << std::endl
        << "    -count                Use the specified values as the number of samples in the x, y and z direction" << std::endl
        << "    -output               Output voxet file-name, MUST NOT contain the .vo extension, this will be added." << std::endl
        << "    -time                 Time stamp to be appended with the output file name, if provided." << std::endl
        << "    -create-spec          Write a standard spec file into the specified file name," << std::endl
        << "                          the cauldron project file must also be specified." << std::endl
        << "    -nullvaluereplace     Replace null values of the property by a given value." << std::endl
        << "    -nobasement           Ignore basement layers." << std::endl
        << "    -singlepropertyheader Writes one header file for each property. (additional to the multiple property header-file)" << std::endl
        << "    -properties           List of cauldron properties for which voxet files are to be generated" << std::endl
        << "    -verbose              Generate some extra output." << std::endl
        << "    -help                 Print this message." << std::endl
        << "    -?                    Print this message." << std::endl
        << "    -usage                Print this message." << std::endl << std::endl;
    exit(-1);
}

// Write the values to the specified file.
//------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::write(const std::string& name,
    const VoxetPropertyGrid& values) const
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

// Correct the endian-ness of the array.
// The voxet format requires that the binary data is written in big-endian format. After this call the numbers will be un-usable in the code.
//------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::correctEndian(VoxetPropertyGrid& values) const
{
    const int nx = values.getGridDescription().getVoxetNodeCount(0);
    const int ny = values.getGridDescription().getVoxetNodeCount(1);
    const int nz = values.getGridDescription().getVoxetNodeCount(2);

    for (int k = 0; k < nz; ++k)
    {
        for (int j = 0; j < ny; ++j)
        {
            for (int i = 0; i < nx; ++i)
            {
                values(i, j, k) = converters::correctEndian(values(i, j, k));
            }
        }
    }
}

// Write the ascii voxet header (vo-file)
//------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::writeVOheader(ofstream& file,
    const GridDescription& gridDescription,
    const std::string& outputFileName) const
{
    file << "GOCAD Voxet 1.0" << std::endl;
    file << "HEADER" << std::endl;
    file << "{" << std::endl;
    file << "name:" << outputFileName << std::endl;
    file << "}" << std::endl;

    file << "GOCAD_ORIGINAL_COORDINATE_SYSTEM" << std::endl;
    file << "NAME Default" << std::endl;
    file << "AXIS_NAME \"X\" \"Y\" \"Z\" " << std::endl;
    file << "AXIS_UNIT \"m\" \"m\" \"m\" " << std::endl;
    file << "ZPOSITIVE Depth" << std::endl;
    file << "END_ORIGINAL_COORDINATE_SYSTEM" << std::endl;

    file << "AXIS_O "
        << gridDescription.getVoxetGridOrigin(0) << "  "
        << gridDescription.getVoxetGridOrigin(1) << "  "
        << gridDescription.getVoxetGridOrigin(2) << "  " << std::endl;


    file << "AXIS_U " << gridDescription.getVoxetGridMaximum(0) - gridDescription.getVoxetGridOrigin(0) << "  " << " 0.0  0.0 " << std::endl;
    file << "AXIS_V  0.0 " << gridDescription.getVoxetGridMaximum(1) - gridDescription.getVoxetGridOrigin(1) << "  " << " 0.0  " << std::endl;
    file << "AXIS_W 0.0 0.0  " << gridDescription.getVoxetGridMaximum(2) - gridDescription.getVoxetGridOrigin(2) << "  " << std::endl;
    file << "AXIS_MIN 0.0 0.0 0.0 " << std::endl;
    file << "AXIS_MAX 1  1  1" << std::endl;

    file << "AXIS_N "
        << gridDescription.getVoxetNodeCount(0) << "  "
        << gridDescription.getVoxetNodeCount(1) << "  "
        << gridDescription.getVoxetNodeCount(2) << "  " << std::endl;

    file << "AXIS_NAME \"X\" \"Y\" \"Z\" " << std::endl;
    file << "AXIS_UNIT \"m\" \"m\" \"m\" " << std::endl;
    file << "AXIS_TYPE even even even" << std::endl;

    file << std::endl;
}

// Write the property information into the voxet header (vo-file)
//------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::writeVOproperty(ofstream& file,
    const int& propertyCount,
    const CauldronProperty* cauldronProperty,
    const std::string& propertyFileName,
    const float& nullValue) const
{
    file << "PROPERTY " << propertyCount << "  \"" << cauldronProperty->getVoxetName() << '"' << std::endl;
    file << "PROPERTY_KIND " << propertyCount << "  \"" << cauldronProperty->getVoxetName() << '"' << std::endl;
    file << "PROPERTY_CLASS " << propertyCount << " \"" << cauldronProperty->getVoxetName() << '"' << std::endl;
    file << "PROPERTY_CLASS_HEADER " << propertyCount << " \"" << cauldronProperty->getVoxetName() << "\" {" << std::endl;
    file << "name:" << cauldronProperty->getVoxetName() << endl << "}" << std::endl;
    file << "PROPERTY_SUBCLASS " << propertyCount << " " << "QUANTITY Float" << std::endl;
    file << "PROP_ORIGINAL_UNIT " << propertyCount << " " << cauldronProperty->getUnits() << std::endl;
    file << "PROP_UNIT " << propertyCount << " " << cauldronProperty->getUnits() << std::endl;
    file << "PROP_ESIZE " << propertyCount << " " << sizeof(float) << std::endl;
    file << "PROP_ETYPE " << propertyCount << " IEEE " << std::endl;
    file << "PROP_NO_DATA_VALUE " << propertyCount << " " << nullValue << std::endl;
    file << "PROP_FILE " << propertyCount << " " << propertyFileName << std::endl;
    file << std::endl;
}

// Write the tail of the  ascii voxet header (vo-file)
//------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::writeVOtail(ofstream& file) const
{
    file << "END" << std::endl;
}

// Computes the property data needed and generates the data files in the votex format
//------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::compute()
{
    std::string projectfileName = getProjectFileName();

    GeoPhysics::ObjectFactory factory;
    std::unique_ptr<GeoPhysics::ProjectHandle> projectHandle(dynamic_cast<GeoPhysics::ProjectHandle*>(OpenCauldronProject(projectfileName, &factory)));
    DerivedProperties::DerivedPropertyManager propertyManager(*projectHandle);

    bool coupledCalculationMode = false;
    bool started = projectHandle->startActivity("cauldron2voxet", projectHandle->getLowResolutionOutputGrid(), false, false, false);

    if (!started)
    {
        exit(1);
    }

    const DataAccess::Interface::SimulationDetails* simulationDetails = projectHandle->getDetailsOfLastSimulation("fastcauldron");
    std::string simulationMode_fastCauldron;

    if (simulationDetails != nullptr)
    {
        simulationMode_fastCauldron = simulationDetails->getSimulatorMode();
        coupledCalculationMode = simulationMode_fastCauldron == "Overpressure" ||
            simulationMode_fastCauldron == "LooselyCoupledTemperature" ||
            simulationMode_fastCauldron == "CoupledHighResDecompaction" ||
            simulationMode_fastCauldron == "CoupledPressureAndTemperature" ||
            simulationMode_fastCauldron == "CoupledDarcy";
    }
    else
    {
        // If this table is not present the assume that the last
        // fastcauldron mode was not pressure mode.
        // This table may not be present because we are running c2e on an old
        // project, before this table was added.
        coupledCalculationMode = false;
    }

    started = projectHandle->initialise(coupledCalculationMode);

    if (!started)
    {
        exit(1);
    }

    started = projectHandle->setFormationLithologies(true, true);

    if (!started)
    {
        exit(1);
    }

    const DataAccess::Interface::Snapshot* snapshot = projectHandle->findSnapshot(m_snapshotTime);

    if (!snapshot)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "No calculations have been made for snapshot time " << m_snapshotTime;
        exit(-1);
    }

    if (m_createVoxetFileName != "")
    {
        ofstream voxetProjectFileStream;

        voxetProjectFileStream.open(m_createVoxetFileName.c_str());
        if (!voxetProjectFileStream.is_open())
        {
            LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open temporary file " << m_createVoxetFileName << ", aborting ....";
            exit(-1);
        }

        createVoxetProjectFile(*projectHandle, propertyManager, voxetProjectFileStream, snapshot, m_propertyList, m_voxetDimensions);
        return;
    }

    VoxetProjectHandle* voxetProject = nullptr;

    if (m_voxetFileName == "")
    {
        ofstream voxetProjectFileStream;

        char tmpVoxetFileName[256];

        snprintf(tmpVoxetFileName, sizeof(tmpVoxetFileName), "/tmp/voxetProjectFile%d", getpid());

        voxetProjectFileStream.open(tmpVoxetFileName);
        if (!voxetProjectFileStream.is_open())
        {
            LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open temporary file " << tmpVoxetFileName << ", aborting ....";
            exit(-1);
        }

        createVoxetProjectFile(*projectHandle, propertyManager, voxetProjectFileStream, snapshot, m_propertyList, m_voxetDimensions);
        voxetProjectFileStream.close();

        voxetProject = new VoxetProjectHandle(tmpVoxetFileName, *projectHandle);
        unlink(tmpVoxetFileName);
    }
    else
    {
        voxetProject = new VoxetProjectHandle(m_voxetFileName, *projectHandle);
    }

    snapshot = projectHandle->findSnapshot(voxetProject->getSnapshotTime());

    // timeStamp was already appended with "_" in its assignment to avoid a condition here on its existence
    string asciiFileName = "BPA2_" + m_outputFileName + m_timeStamp + ".vo";

    if (m_singlePropertyHeader)
    {
        // timeStamp was already appended with "_" in its assignment to avoid a condition here on its existence
        asciiFileName = "BPA2_" + m_outputFileName + "_all" + m_timeStamp + ".vo";
    }
    string binaryFileName = m_outputFileName;

    ofstream asciiOutputFile;

    asciiOutputFile.open(asciiFileName.c_str());

    if (asciiOutputFile.fail())
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open output file " << asciiFileName;
        exit(-1);
    }

    if (projectfileName.length() == 0)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open project file " << projectfileName;
        exit(-1);
    }

    if (m_verbose)
    {
        LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << "Using snapshot " << setprecision(10) << snapshot->getTime();
    }

    const DataAccess::Interface::Property* depthProperty = projectHandle->findProperty("Depth");

    if (!depthProperty)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not find the Depth property in the project file. "
            << "Are you sure the project file contains output data?";
        exit(-1);
    }

    int propertyCount = 1;

    const GridDescription& gridDescription = voxetProject->getGridDescription();

    VoxetCalculator vc(*projectHandle, propertyManager, voxetProject->getGridDescription(), m_propertyNullValueReplaceLookup);

    if (m_useBasement && m_verbose) LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << "Using basement";

    vc.setDepthProperty(depthProperty);


    // Array used to store interpolated values, this will then be output to file.
    VoxetPropertyGrid interpolatedProperty(voxetProject->getGridDescription());

    asciiOutputFile.flags(ios::fixed);

    writeVOheader(asciiOutputFile, gridDescription, m_outputFileName);

    std::string propertyName;

    CauldronPropertyList::iterator cauldronPropIter;
    for (cauldronPropIter = voxetProject->cauldronPropertyBegin(); cauldronPropIter != voxetProject->cauldronPropertyEnd(); ++cauldronPropIter)
    {
        const DataAccess::Interface::Property* property = (*cauldronPropIter)->getProperty();
        if (!property)
        {
            LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Unknown property: " << (*cauldronPropIter)->getCauldronName();
            continue;
        }
        propertyName = property->getName();
        // Do not calculate the TwoWayTime after the decompaction run as it throws exception unlike other properties when attempted to compute
        // as the property can not be computed after just the decompaction run
        if (simulationMode_fastCauldron == "HydrostaticDecompaction" && (propertyName == "TwoWayTime")) continue;

        /// The below piece of code was disabled for BPA2 scenarios as with the code being active, it calculates only the properties
        /// which are listed in 3DTimeIoTbl. In BPA-2, only the fundamental properties are written to the table instead of
        /// all the properties as in BPA-legacy. So, for compability with BPA-2, this check had to be removed to enable the computations
        /// of the all properties specified in the spec file.
#if 0
      /// Check if the property has values (only for 2016.11 release. Remove this check to enable DerivedProperty calculation)
      /// This check was introduced in BPA-legacy to disable the calculations of properties which are not listed in 3DTimeIoTbl of
      /// the project3d file. User defined properties with user specifying the equations to calculate such properties which are
      /// based on other properties are one such example. The functionality was disabled in the git commit 72d1d9f4f4778c61e1a6b19b9ceee98536bdb4cb
      /// leading to this check. Refer SpecFileVersion.h file in the build for details.

        const PropertyValueList* propertyValueListAvailable = projectHandle->getPropertyValues(FORMATION, property, snapshot, nullptr, nullptr, nullptr, VOLUME);
        unsigned int propertiesSize = propertyValueListAvailable->size();
        delete propertyValueListAvailable;
        if (propertiesSize == 0)
        {
            continue;
        }
#endif

        vc.useBasement() = m_useBasement;
        // Check if the property is computable at basement
        if (m_useBasement)
        {
            vc.useBasement() = converters::isBasementProperty(property->getName());
        }

        AbstractDerivedProperties::FormationPropertyList propertyValueList(propertyManager.getFormationProperties(property, snapshot, vc.useBasement()));

        // Could just ask if property is computable.
        if (propertyValueList.size() == 0)
        {
            continue;
        }

        if (m_verbose)
        {
            LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << " Adding cauldron property: " << property->getName();
        }

        if ((*cauldronPropIter)->getVoxetOutput())
        {

            vc.addProperty(property);

            if (vc.computeInterpolators(snapshot, m_verbose) == -1)
            {
                LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Are there any results in the project? ";
                exit(-1);
            }

            // timeStamp was already appended with "_" in its assignment to avoid a condition here on its existence
            std::string propertyFileName = "BPA2_" + binaryFileName + "_" + (*cauldronPropIter)->getCauldronName() + m_timeStamp + "@@";

            writeVOproperty(asciiOutputFile, propertyCount, *cauldronPropIter, propertyFileName, vc.getNullValue(property));

            vc.computeProperty(*cauldronPropIter, interpolatedProperty, m_verbose);
            correctEndian(interpolatedProperty);
            write(propertyFileName, interpolatedProperty);
            ++propertyCount;

            if (m_verbose)
            {
                LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << " deleting interpolators for property: " << property->getName();
            }

            if (m_singlePropertyHeader)
            {
                std::ofstream asciiHeaderOutputFile;
                std::string asciiHeaderFileName = "BPA2_" + m_outputFileName + "_" + (*cauldronPropIter)->getCauldronName() + m_timeStamp + ".vo";

                asciiHeaderOutputFile.open(asciiHeaderFileName.c_str());

                if (asciiHeaderOutputFile.fail())
                {
                    LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open output file " << asciiHeaderFileName;
                    exit(-1);
                }
                asciiHeaderOutputFile.flags(std::ios::fixed);

                writeVOheader(asciiHeaderOutputFile, gridDescription, m_outputFileName);

                writeVOproperty(asciiHeaderOutputFile, 1, *cauldronPropIter, propertyFileName, vc.getNullValue(property));

                writeVOtail(asciiHeaderOutputFile);

                asciiHeaderOutputFile.close();
            }
            vc.deleteProperty(property);
        }
    }

    writeVOtail(asciiOutputFile);
    asciiOutputFile.close();

    if (m_debug)
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Project closed";
}

// Print to stdout a default voxet file based on the cauldron project file that has been input.
//------------------------------------------------------------//
void cauldron_to_voxet::Cauldron2VoxetConverter::createVoxetProjectFile(const DataAccess::Interface::ProjectHandle& cauldronProject, 
    DerivedProperties::DerivedPropertyManager& propertyManager, 
    ostream& outputStream, const DataAccess::Interface::Snapshot* snapshot, 
    const std::vector<std::string>& propertyList, 
    const VoxetAttributes& voxetDimensions) const
{
    const database::DataSchema* voxetSchema = database::createVoxetSchema();
    database::Database* database = database::Database::CreateFromSchema(*voxetSchema);
    database::Table* table;
    database::Record* record;
    const DataAccess::Interface::Grid* grid;

    table = database->getTable("CauldronPropertyIoTbl");

    // Looping over the list of cauldron properties available for conversion to voxet
    for (const auto& cauldronPropertyObj : m_cauldronPropertiesForVoxet)
    {
        const DataAccess::Interface::Property* property = cauldronProject.findProperty(cauldronPropertyObj.propertyName);
        if (property)
        {
            // propertyList contains the list of cauldron properties provided by user at command line for conversion to voxet
            // if user has not used the '-properties' argument, then generate the voxets for all the properties
            if (propertyList.empty())
            {
                record = table->createRecord();
                database::setCauldronPropertyName(record, cauldronPropertyObj.propertyName);
                database::setVoxetPropertyName(record, cauldronPropertyObj.outputPropertyName);
                database::setOutputPropertyUnits(record, cauldronPropertyObj.unit);
                database::setConversionFactor(record, cauldronPropertyObj.conversion);
                database::setVoxetOutput(record, 1);

            }
            // User has provided the cauldron properties list to be converted to voxets
            else
            {
                // Convert only the properties provided by the user to voxet
                // Out of all the properties available for conversion, check for which property user is asking for conversion to voxet
                auto it = std::find(propertyList.begin(), propertyList.end(), cauldronPropertyObj.propertyName);
                if (it != propertyList.end())
                {
                    record = table->createRecord();
                    database::setCauldronPropertyName(record, cauldronPropertyObj.propertyName);
                    database::setVoxetPropertyName(record, cauldronPropertyObj.outputPropertyName);
                    database::setOutputPropertyUnits(record, cauldronPropertyObj.unit);
                    database::setConversionFactor(record, cauldronPropertyObj.conversion);
                    database::setVoxetOutput(record, 1);
                }
            }
        }
    }

    table = database->getTable("SnapshotTimeIoTbl");

    record = table->createRecord();
    database::setSnapshotTime(record, snapshot->getTime());

    const DataAccess::Interface::Property* depthProperty = cauldronProject.findProperty("Depth");

    if (!depthProperty)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not find the Depth property in the project file. "
            << "Are you sure the project file contains output data?";
        return;
    }

    /// Basin's Total Depth Computations for BPA-2 Scenarios
    /// for calculating the voxet grid parameters in depth direction

    DataAccess::Interface::SurfaceList* surfaces = cauldronProject.getSurfaces();
    const DataAccess::Interface::Surface* bottomSurface = surfaces->back();

    AbstractDerivedProperties::SurfacePropertyPtr abstractBottomDepthPropertyValue = propertyManager.getSurfaceProperty(depthProperty, snapshot, bottomSurface);
    auto bottomDepthPropertyValue = dynamic_pointer_cast<const AbstractDerivedProperties::FormationPropertyAtSurface>(abstractBottomDepthPropertyValue);

    if (bottomDepthPropertyValue == nullptr)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for bottom surface " << bottomSurface->getName()
            << " at snapshot " << snapshot->getTime() << " is not available.";
        return;
    }

    const DataModel::AbstractGrid* bottomDepthGrid = abstractBottomDepthPropertyValue->getGrid();
    if (bottomDepthGrid == nullptr)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for bottom surface " << bottomSurface->getName()
            << " at snapshot " << snapshot->getTime() << " is not available.";
        return;
    }
    int minI = bottomDepthGrid->firstI(false);
    int maxI = bottomDepthGrid->lastI(false);
    int minJ = bottomDepthGrid->firstJ(false);
    int maxJ = bottomDepthGrid->lastJ(false);

    // Computing the maximum value of depth for the bottom formation grid
    double minDepthValInGrid = std::numeric_limits< double >::max();
    double maxDepthValInGrid = -std::numeric_limits< double >::max();

    for (int i = minI; i <= maxI; ++i)
    {
        for (int j = minJ; j <= maxJ; ++j)
        {
            double value = bottomDepthPropertyValue->get(i, j);
            // Ignoring Undefined Value = 99999 at nodes
            if(!NumericFunctions::isEqual(value,99999.0,1e-15) && (value > maxDepthValInGrid))
                maxDepthValInGrid = value;                
        }
    }
    // Maximum depth of the basin is returned by the maximum depth of the bottom formation
    double maxBasinDepth = maxDepthValInGrid;

    const DataModel::AbstractGrid* topDepthGrid = nullptr;

    std::shared_ptr<const AbstractDerivedProperties::FormationPropertyAtSurface> topDepthPropertyValue = nullptr;


    DataAccess::Interface::SurfaceList::iterator surfaceIter;
    for (surfaceIter = surfaces->begin(); topDepthGrid == nullptr && surfaceIter != surfaces->end(); ++surfaceIter)
    {
        const DataAccess::Interface::Surface* topSurface = *surfaceIter;

        AbstractDerivedProperties::SurfacePropertyPtr abstractTopDepthPropertyValue = propertyManager.getSurfaceProperty(depthProperty, snapshot, topSurface);
        topDepthPropertyValue = dynamic_pointer_cast<const AbstractDerivedProperties::FormationPropertyAtSurface>(abstractTopDepthPropertyValue);

        if (topDepthPropertyValue == nullptr)
        {
            LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for top surface " << topSurface->getName()
                << " at snapshot " << snapshot->getTime() << " is not available.";
            continue;
        }
        topDepthGrid = abstractTopDepthPropertyValue->getGrid();
        break;
    }

    if (topDepthGrid == nullptr)
    {
        LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for top surface" << " is not available.";
        return;
    }

    minI = topDepthGrid->firstI(false);
    maxI = topDepthGrid->lastI(false);
    minJ = topDepthGrid->firstJ(false);
    maxJ = topDepthGrid->lastJ(false);

    // Computing the minimum value of depth for the basin
    for (int i = minI; i <= maxI; ++i)
    {
        for (int j = minJ; j <= maxJ; ++j)
        {
            double value = topDepthPropertyValue->get(i, j);
            // Ignoring Undefined Value = 99999 at nodes
            if(!NumericFunctions::isEqual(value,99999.0,1e-15) && (value < minDepthValInGrid))
                minDepthValInGrid = value;
        }
    }

    // Minimum depth of the basin is the minimum depth of the top formation
    double minBasinDepth = minDepthValInGrid;
    // Low resolution grid is the actual simulation obtained after subsampling
    grid = cauldronProject.getLowResolutionOutputGrid();
    table = database->getTable("VoxetGridIoTbl");

    const double deltaK = 100;

    record = table->createRecord();
    database::setVoxetOriginX(record, converters::selectDefined(MAXDOUBLE, voxetDimensions.originX, grid->minI()));
    database::setVoxetOriginY(record, converters::selectDefined(MAXDOUBLE, voxetDimensions.originY, grid->minJ()));
    database::setVoxetOriginZ(record, converters::selectDefined(MAXDOUBLE, voxetDimensions.originZ, converters::roundoff((minBasinDepth - 100), 3)));
    database::setVoxetDeltaX(record, converters::selectDefined(MAXDOUBLE, voxetDimensions.deltaX, grid->deltaI()));
    database::setVoxetDeltaY(record, converters::selectDefined(MAXDOUBLE, voxetDimensions.deltaY, grid->deltaJ()));
    database::setVoxetDeltaZ(record, converters::selectDefined(MAXDOUBLE, voxetDimensions.deltaZ, deltaK));
    database::setNumberOfVoxetNodesX(record, (int)converters::selectDefined(MAXDOUBLE, voxetDimensions.countX, (double)grid->numI()));
    database::setNumberOfVoxetNodesY(record, (int)converters::selectDefined(MAXDOUBLE, voxetDimensions.countY, (double)grid->numJ()));
    database::setNumberOfVoxetNodesZ(record, (int)converters::selectDefined(MAXDOUBLE, voxetDimensions.countZ, ((maxBasinDepth - minBasinDepth) / deltaK) + 3));

    // Now write the stream to stdout.
    database->saveToStream(outputStream);
}

// Check if any flag to display help is provided at the command line
//------------------------------------------------------------//
void cauldron_to_voxet::readHelpFlagsFromCmdLine(std::vector<std::string>& argList)
{
    std::vector<std::string>::iterator it;

    it = std::find(argList.begin(), argList.end(), std::string("-help"));
    if (it != argList.end())
    {
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(" Standard usage.", (argList.at(0)).c_str());
        exit(-1);
    }
    it = std::find(argList.begin(), argList.end(), std::string("-?"));
    if (it != argList.end())
    {
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(" Standard usage.", (argList.at(0)).c_str());
        exit(-1);
    }
    it = std::find(argList.begin(), argList.end(), std::string("-usage"));
    if (it != argList.end())
    {
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(" Standard usage.", (argList.at(0)).c_str());
        exit(-1);
    }
}

// Read the string next to the 'parameterToRead' in the 'outString'
//------------------------------------------------------------//
void cauldron_to_voxet::readSingleStringFromCmdLine(std::vector<std::string>& argList, const std::string& parameterToRead, std::string& outString)
{
    std::vector<std::string>::iterator it;

    it = std::find(argList.begin(), argList.end(), parameterToRead);
    if (it != argList.end() && (it + 1) != argList.end()) // found the parameterToRead and it is not the last element
    {
        outString = *(it + 1);
        argList.erase(it, it + 2);
    }
    else if (it != argList.end() && (it + 1) == argList.end()) // found the parameterToRead and it is the last element
    {
        std::string message = std::string("Argument for '") + parameterToRead + std::string("' is missing");
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(message.c_str(), (argList.at(0)).c_str());
        exit(-1);
    }
    else if (it == argList.end())
    {
        outString = "";
    }

}

// Read the Voxet grid parameter 'parameterToRead' into the floating point numbers 'voxetParamX', 'voxetParamY','voxetParamZ'
//------------------------------------------------------------//
void cauldron_to_voxet::readVoxetAttribsFromCmdLine(std::vector<std::string>& argList, const std::string& parameterToRead, double& voxetParamX, double& voxetParamY, double& voxetParamZ)
{
    std::vector<std::string>::iterator it;

    it = std::find(argList.begin(), argList.end(), parameterToRead);
    if (it != argList.end() && (it + 1) != argList.end()) // found the parameterToRead and it is not the last element
    {
        std::string tempString = *(it + 1);
        char* c_voxetParam = &tempString[0];
        char* c_voxetParamX;
        char* c_voxetParamY;
        char* c_voxetParamZ;

        converters::splitString(c_voxetParam, ',', c_voxetParamX, c_voxetParamY, c_voxetParamZ);

        if (c_voxetParamX) voxetParamX = atof(c_voxetParamX);
        if (c_voxetParamY) voxetParamY = atof(c_voxetParamY);
        if (c_voxetParamZ) voxetParamZ = atof(c_voxetParamZ);

        argList.erase(it, it + 2);
    }
    else if (it != argList.end() && (it + 1) == argList.end()) // found the parameterToRead and it is the last element
    {
        std::string message = std::string("Argument for '") + parameterToRead + std::string("' is missing");
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage(message.c_str(), (argList.at(0)).c_str());
        exit(-1);
    }
}

// Read the 'nullvaluereplace' and the properties provided
//------------------------------------------------------------//
void cauldron_to_voxet::readNullValueReplaceFromCmdLine(std::vector<std::string>& argList, cauldron_to_voxet::ArgumentList& arguments)
{
    std::vector<std::string>::iterator it;

    // -nullvaluereplace
    it = std::find(argList.begin(), argList.end(), std::string("-nullvaluereplace"));

    auto itLocaNullValue = it;
    int count = 0;

    if (it != argList.end() && (it + 1) != argList.end()) // found the '-nullvaluereplace' and it is not the last element
    {
        ++it;
        std::string nextStringInTheList = *it;
        // condition: There is a next string after '-nullvaluereplace' and it does not start with '-'
        while (it != argList.end() && nextStringInTheList.compare(0, 1, "-"))
        {

            char* c_nullValueReplaceOption = &nextStringInTheList[0];
            char* nullValueReplaceName;
            char* nullValueReplaceValue;
            char* tmp;
            converters::splitString(c_nullValueReplaceOption, ',', nullValueReplaceName, nullValueReplaceValue, tmp);
            if (!nullValueReplaceName || !nullValueReplaceValue)
            {
                cauldron_to_voxet::Cauldron2VoxetConverter::showUsage("Argument for '-nullvalueeplace' wrong format", arguments.argv0);
                exit(-1);
            }
            arguments.propertyNullValueReplaceLookup.insert(std::pair<std::string, double>(std::string(nullValueReplaceName),
                atof(nullValueReplaceValue)));
            ++count;
            ++it;
            if (it != argList.end())
            {
                nextStringInTheList = *it;
            }
        }
        argList.erase(itLocaNullValue, itLocaNullValue + count + 1);
    }
    else if (it != argList.end() && (it + 1) == argList.end()) // found the '-nullvaluereplace' and it is the last element
    {
        cauldron_to_voxet::Cauldron2VoxetConverter::showUsage("Argument for '-nullvaluereplace' is missing", (argList.at(0)).c_str());
        exit(-1);
    }

}

// Returns true if 'parameterToRead' is provided at the command line
//------------------------------------------------------------//
bool cauldron_to_voxet::readBoolFlagsFromCmdLine(std::vector<std::string>& argList, const std::string& parameterToRead)
{
    std::vector<std::string>::iterator it;
    it = std::find(argList.begin(), argList.end(), parameterToRead);
    if (it != argList.end())
    {
        argList.erase(it);
        return true;
    }
    else
    {
        return false;
    }  
}
