#include "VoxetUtils.h"


void write(const std::string& name,
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


float correctEndian(const float x)
{

	union ByteConverter
	{
		float value;
		char bytes[4];
	};

	ByteConverter bc;
	ByteConverter result;

	bc.value = x;
	result.bytes[0] = bc.bytes[3];
	result.bytes[1] = bc.bytes[2];
	result.bytes[2] = bc.bytes[1];
	result.bytes[3] = bc.bytes[0];

	return result.value;
}

void correctEndian(VoxetPropertyGrid& values)
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
				values(i, j, k) = correctEndian(values(i, j, k));
			}
		}
	}
}

bool splitString(char* string, char separator, char*& firstPart, char*& secondPart, char*& thirdPart)
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

double selectDefined(double undefinedValue, double preferred, double alternative)
{
	return (preferred != undefinedValue ? preferred : alternative);
}


void createVoxetProjectFile(const Interface::ProjectHandle& cauldronProject,
	DerivedProperties::DerivedPropertyManager& propertyManager,
	ostream& outputStream, const Snapshot* snapshot)
{
	char* propertyNames[] =
	{
	   "Depth",
	   "Pressure", "OverPressure", "HydroStaticPressure", "LithoStaticPressure",
	   "Temperature", "Vr",
	   "Ves", "MaxVes",
	   "Porosity", "Permeability", "BulkDensity",
	   "Velocity", "TwoWayTime",
	   ""
	};

	char* units[] =
	{
	   "m",
	   "MPa", "MPa", "MPa", "MPa",
	   "degC", "percent",
	   "Pa", "Pa",
	   "percent", "mD", "kg/m^3",
	   "m/s", "ms",
	   ""
	};

	double conversions[] =
	{
	   1,
	   1, 1, 1, 1,
	   1, 1,
	   1, 1,
	   1, 1, 1,
	   1, 1,
	   0
	};

	char* outputPropertyNames[] =
	{
	   "BPA_Depth",
	   "BPA_Pressure", "BPA_OverPressure", "BPA_HydrostaticPressure", "BPA_LithoStaticPressure",
	   "BPA_Temperature", "BPA_Vr",
	   "BPA_Ves", "BPA_MaxVes",
	   "BPA_Porosity", "BPA_Permeability", "BPA_BulkDensity",
	   "BPA_Velocity", "BPA_TwoWayTime",
	   ""
	};

	const database::DataSchema* voxetSchema = database::createVoxetSchema();
	database::Database* database = database::Database::CreateFromSchema(*voxetSchema);
	database::Table* table;
	database::Record* record;
	const Interface::Grid* grid;

	//------------------------------------------------------------//

	table = database->getTable("CauldronPropertyIoTbl");
	int p;
	for (p = 0; strlen(propertyNames[p]) != 0; ++p)
	{
		const Property* property = cauldronProject.findProperty(propertyNames[p]);
		if (property)
		{
			record = table->createRecord();
			database::setCauldronPropertyName(record, propertyNames[p]);
			database::setVoxetPropertyName(record, outputPropertyNames[p]);
			database::setOutputPropertyUnits(record, units[p]);
			database::setConversionFactor(record, conversions[p]);
			database::setVoxetOutput(record, 1);
		}
	}

	//------------------------------------------------------------//

	table = database->getTable("SnapshotTimeIoTbl");

	record = table->createRecord();
	database::setSnapshotTime(record, snapshot->getTime());

	//------------------------------------------------------------//

	const Property* depthProperty = cauldronProject.findProperty("Depth");

	if (!depthProperty)
	{
		LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not find the Depth property in the project file. "
			<< "Are you sure the project file contains output data?";
		return;
	}

	Interface::SurfaceList* surfaces = cauldronProject.getSurfaces();
	const Interface::Surface* bottomSurface = surfaces->back();

	AbstractDerivedProperties::SurfacePropertyPtr abstractBottomDepthPropertyValue = propertyManager.getSurfaceProperty(depthProperty, snapshot, bottomSurface);
	auto bottomDepthPropertyValue = dynamic_pointer_cast<const DerivedProperties::PrimarySurfaceProperty>(abstractBottomDepthPropertyValue);

	if (bottomDepthPropertyValue == nullptr)
	{
		LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for bottom surface " << bottomSurface->getName()
			<< " at snapshot " << snapshot->getTime() << " is not available.";
		return;
	}

	const GridMap* bottomDepthGridMap = nullptr;
	const GridMap* topDepthGridMap = nullptr;

	bottomDepthGridMap = bottomDepthPropertyValue.get()->getGridMap();

	if (bottomDepthGridMap == nullptr)
	{
		LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for bottom surface " << bottomSurface->getName()
			<< " at snapshot " << snapshot->getTime() << " is not available.";
		return;
	}

	Interface::SurfaceList::iterator surfaceIter;
	for (surfaceIter = surfaces->begin(); topDepthGridMap == nullptr && surfaceIter != surfaces->end(); ++surfaceIter)
	{
		const Interface::Surface* topSurface = *surfaceIter;

		AbstractDerivedProperties::SurfacePropertyPtr abstractTopDepthPropertyValue = propertyManager.getSurfaceProperty(depthProperty, snapshot, topSurface);
		auto topDepthPropertyValue = dynamic_pointer_cast<const DerivedProperties::PrimarySurfaceProperty>(abstractTopDepthPropertyValue);

		if (topDepthPropertyValue == nullptr)
		{
			LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for top surface " << topSurface->getName()
				<< " at snapshot " << snapshot->getTime() << " is not available.";
			continue;
		}
		topDepthGridMap = topDepthPropertyValue.get()->getGridMap();
		break;
	}

	if (topDepthGridMap == nullptr)
	{
		LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Depth property for top surface" << " is not available.";
		return;
	}

	double minimumDepth;
	double maximumDepth;
	double dummyDepth;

	topDepthGridMap->getMinMaxValue(minimumDepth, dummyDepth);
	bottomDepthGridMap->getMinMaxValue(dummyDepth, maximumDepth);

	//------------------------------------------------------------//

	grid = cauldronProject.getLowResolutionOutputGrid();
	table = database->getTable("VoxetGridIoTbl");

	const double deltaK = 100;

	record = table->createRecord();
	database::setVoxetOriginX(record, selectDefined(MAXDOUBLE, originX, grid->minI()));
	database::setVoxetOriginY(record, selectDefined(MAXDOUBLE, originY, grid->minJ()));
	database::setVoxetOriginZ(record, selectDefined(MAXDOUBLE, originZ, minimumDepth - 100));
	database::setVoxetDeltaX(record, selectDefined(MAXDOUBLE, deltaX, grid->deltaI()));
	database::setVoxetDeltaY(record, selectDefined(MAXDOUBLE, deltaY, grid->deltaJ()));
	database::setVoxetDeltaZ(record, selectDefined(MAXDOUBLE, deltaZ, deltaK));
	database::setNumberOfVoxetNodesX(record, (int)selectDefined(MAXDOUBLE, countX, (double)grid->numI()));
	database::setNumberOfVoxetNodesY(record, (int)selectDefined(MAXDOUBLE, countY, (double)grid->numJ()));
	database::setNumberOfVoxetNodesZ(record, (int)selectDefined(MAXDOUBLE, countZ, ((maximumDepth - minimumDepth) / deltaK) + 3));

	//------------------------------------------------------------//

	// Now write the stream to stdout.
	database->saveToStream(outputStream);
}

void writeVOheader(ofstream& file,
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

void writeVOproperty(ofstream& file,
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
	file << "PROP_NO_DATA_VALUE " << propertyCount << " " << nullValue << endl;
	file << "PROP_FILE " << propertyCount << " " << propertyFileName << endl;
	file << endl;
}

void writeVOtail(ofstream& file)
{
	file << "END" << endl;
}

bool isBasementProperty(const std::string& property)
{
	// this list of properties is not exhaustive; out of the default properties listed in the spec file,
	// these are the properties which can be computed at the sediment+basement
	if (property == "BulkDensity" || property == "Velocity" || property == "Depth" ||
		property == "Temperature" || property == "LithoStaticPressure") return true;
	else return false;
}
