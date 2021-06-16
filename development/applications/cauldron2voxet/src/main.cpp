//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VoxetUtils.h"

bool useBasement = true;
bool verbose = false;
bool singlePropertyHeader = false;
bool debug = false;

static char * argv0 = nullptr;

namespace
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
}


/// Print to stdout a default voxet file based on the cauldron project file that has been input.
void createVoxetProjectFile (const DataAccess::Interface::ProjectHandle &cauldronProject,
                              DerivedProperties::DerivedPropertyManager& propertyManager,
                              ostream & outputStream, const DataAccess::Interface::Snapshot * snapshot, const std::vector<string>& propertyList );

static void showUsage(const char* message)
{
    cerr << endl;
    if (message)
    {
        cerr << argv0 << ": " << message << endl;
    }

    cerr << "Usage (Options may be abbreviated): " << endl
        << argv0 << "    -project <cauldron-project-file>" << endl
        << "                  [-spec <spec-file>]" << endl
        << "                  [-snapshot <age>]" << endl
        << "                  [-origin <originX>,<originY>,<originZ>]" << endl
        << "                  [-delta <deltaX>,<deltaY>,<deltaZ>]" << endl
        << "                  [-count <countX>,<countY>,<countZ>]" << endl
        << "                  [-output <output-file-name>]" << endl
        << "                  [-time <time-stamp>]" << endl
        << "                  [-create-spec <spec-file>]" << endl
        << "                  [-nullvaluereplace <PropertyName,Value> [<PropertyName,Value>] [...]]" << endl
        << "                  [-nobasement]" << endl
        << "                  [-singlepropertyheader]" << endl
        << "                  [-properties]" << endl
        << "                  [-verbose]" << endl
        << "                  [-help]" << endl
        << "                  [-?]" << endl
        << "                  [-usage]" << endl
        << endl
        << "    -project              The cauldron project file." << endl
        << "    -spec                 Use the specified spec file. Use a standard spec file if missing." << endl
        << "    -snapshot             Use the specified snapshot age. Not valid in conjunction with '-spec'," << endl
        << "    -origin               Use the specified coordinates as the origin of the sample cube" << endl
        << "    -delta                Use the specified values as the sampling distance in the x, y and z direction" << endl
        << "    -count                Use the specified values as the number of samples in the x, y and z direction" << endl
        << "    -output               Output voxet file-name, MUST NOT contain the .vo extension, this will be added." << endl
        << "    -time                 Time stamp to be appended with the output file name, if provided." << endl
        << "    -create-spec          Write a standard spec file into the specified file name," << endl
        << "                          the cauldron project file must also be specified." << endl
        << "    -nullvaluereplace     Replace null values of the property by a given value." << endl
        << "    -nobasement           Ignore basement layers." << endl
        << "    -singlepropertyheader Writes one header file for each property. (additional to the multiple property header-file)" << endl
        << "    -properties           List of cauldron properties for which voxet files are to be generated" << endl
        << "    -verbose              Generate some extra output." << endl
        << "    -help                 Print this message." << endl
        << "    -?                    Print this message." << endl
        << "    -usage                Print this message." << endl << endl;
    exit(-1);
}


int main (int argc, char ** argv)
{
   string projectFileName;
   string voxetFileName;
   string createVoxetFileName;
   string outputFileName;
   string timeStamp = "";
   string properties = "";
   std::vector<string> propertyList;

   std::map<std::string, double > propertyNullValueReplaceLookup = std::map<std::string, double >();

   if ((argv0 = strrchr (argv[0], '/')) != nullptr)
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }

   double snapshotTime = 0;
   int arg;

   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-output", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-output' is missing");
            return -1;
         }
         outputFileName = argv[++arg];
      }
      else if (strncmp(argv[arg], "-time", Max(2, strlen(argv[arg]))) == 0)
      {
          if (arg + 1 >= argc)
          {
              showUsage("Argument for '-time' is missing");
              return -1;
          }
          timeStamp = argv[++arg];
          // "_" is appended here to avoid a condition on existence of timeStamp at later part of the code
          timeStamp = "_" + timeStamp;
      }
      else if (strncmp(argv[arg], "-properties", Max(2, strlen(argv[arg]))) == 0)
      {
          if (arg + 1 >= argc)
          {
              showUsage("Argument for '-properties' is missing");
              return -1;
          }
          properties = argv[++arg];
          VoxetUtils::fetchPropertyList(properties, propertyList);
      }
      else if (strncmp (argv[arg], "-project", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-project' is missing");
            return -1;
         }
         projectFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-spec", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-spec' is missing");
            return -1;
         }
         voxetFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-snapshot", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-snapshot' is missing");
            return -1;
         }
         snapshotTime = atof (argv[++arg]);
      }
      else if (strncmp (argv[arg], "-origin", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-origin' is missing");
            return -1;
         }
         char * c_origins = argv[++arg];
         char * c_originX;
         char * c_originY;
         char * c_originZ;

         VoxetUtils::splitString (c_origins, ',', c_originX, c_originY, c_originZ);

         if (c_originX) originX = atof (c_originX);
         if (c_originY) originY = atof (c_originY);
         if (c_originZ) originZ = atof (c_originZ);
      }
      else if (strncmp (argv[arg], "-delta", Max (4, strlen (argv[arg]))) == 0)
      {
          if (arg + 1 >= argc)
          {
              showUsage ("Argument for '-delta' is missing");
              return -1;
          }
          char * c_deltas = argv[++arg];
          char * c_deltaX;
          char * c_deltaY;
          char * c_deltaZ;

          VoxetUtils::splitString (c_deltas, ',', c_deltaX, c_deltaY, c_deltaZ);

          if (c_deltaX) deltaX = atof (c_deltaX);
          if (c_deltaY) deltaY = atof (c_deltaY);
          if (c_deltaZ) deltaZ = atof (c_deltaZ);
      }
      else if (strncmp (argv[arg], "-count", Max (2, strlen (argv[arg]))) == 0)
      {
          if (arg + 1 >= argc)
          {
              showUsage ("Argument for '-count' is missing");
              return -1;
          }
          char * c_counts = argv[++arg];
          char * c_countX;
          char * c_countY;
          char * c_countZ;

          VoxetUtils::splitString (c_counts, ',', c_countX, c_countY, c_countZ);

          if (c_countX) countX = atof (c_countX);
          if (c_countY) countY = atof (c_countY);
          if (c_countZ) countZ = atof (c_countZ);
      }
      else if (strncmp (argv[arg], "-nullvaluereplace", Max (5, strlen (argv[arg]))) == 0)
      {
          if (arg + 1 >= argc)
          {
              showUsage ("Argument for '-nullvaluereplace' is missing");
              return -1;
          }
          while (arg+1<argc && strncmp( argv[arg+1],"-",1)!=0 )
          {
              char * c_nullValueReplaceOption = argv[++arg];
              char * nullValueReplaceName;
              char * nullValueReplaceValue;
              char * tmp;
              VoxetUtils::splitString (c_nullValueReplaceOption,',',nullValueReplaceName,nullValueReplaceValue,tmp);
              if(!nullValueReplaceName || !nullValueReplaceValue)
              {
                  showUsage ("Argument for '-nullvalueeplace' wrong format");
                  return -1;
              }
              propertyNullValueReplaceLookup.insert(std::pair<std::string,double>(std::string(nullValueReplaceName),
              atof (nullValueReplaceValue)));
          }
      }
      else if (strncmp (argv[arg], "-debug", Max (4, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if ((strncmp (argv[arg], "-help",  Max (2, strlen (argv[arg]))) == 0) ||
             (strncmp (argv[arg], "-?",     Max (2, strlen (argv[arg]))) == 0) ||
         (strncmp (argv[arg], "-usage", Max (2, strlen (argv[arg]))) == 0))
      {
         showUsage (" Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-nobasement", Max (4, strlen (argv[arg]))) == 0)
      {
         useBasement = false;
      }
      else if (strncmp (argv[arg], "-verbose", Max (4, strlen (argv[arg]))) == 0)
      {
         verbose = true;
      }
      else if (strncmp (argv[arg], "-singlepropertyheader", Max (4, strlen (argv[arg]))) == 0)
      {
         singlePropertyHeader = true;
      }
      else if (strncmp (argv[arg], "-create-spec", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-create-spec' is missing");
            return -1;
         }
         createVoxetFileName = argv[++arg];
      }
      else
      {
          char errorMessage[256];
          snprintf (errorMessage, sizeof (errorMessage), " Illegal argument: %s", argv[arg]);
          showUsage (errorMessage);
          return -1;
      }
   }

   if (projectFileName == "")
   {
      showUsage ("No project file specified");
      return -1;
   }

   if (outputFileName == "")
   {
      size_t dotPos = projectFileName.find (".project");
      outputFileName = projectFileName.substr (0, dotPos);
   }

   GeoPhysics::ObjectFactory factory;
   std::unique_ptr<GeoPhysics::ProjectHandle> projectHandle( dynamic_cast< GeoPhysics::ProjectHandle* >( OpenCauldronProject( projectFileName, &factory ) ) );
   DerivedProperties::DerivedPropertyManager propertyManager ( *projectHandle );

   bool coupledCalculationMode = false;
   bool started = projectHandle->startActivity ( "cauldron2voxet", projectHandle->getLowResolutionOutputGrid (), false, false, false );

   if ( !started )
   {
      return 1;
   }

   const DataAccess::Interface::SimulationDetails* simulationDetails = projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );
   std::string simulationMode_fastCauldron;

   if ( simulationDetails != nullptr )
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

   started = projectHandle->initialise ( coupledCalculationMode );

   if ( !started )
   {
      return 1;
   }

   started = projectHandle->setFormationLithologies ( true, true );

   if ( !started )
   {
      return 1;
   }


   const DataAccess::Interface::Snapshot *snapshot = projectHandle->findSnapshot (snapshotTime);

   if (!snapshot)
   {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "No calculations have been made for snapshot time " << snapshotTime;
      return -1;
   }

   if (createVoxetFileName != "")
   {
      ofstream voxetProjectFileStream;

      voxetProjectFileStream.open (createVoxetFileName.c_str ());
      if (!voxetProjectFileStream.is_open ())
      {
         LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open temporary file " << createVoxetFileName << ", aborting ....";
         return -1;
      }

      createVoxetProjectFile (*projectHandle, propertyManager, voxetProjectFileStream, snapshot, propertyList);
      return 0;
   }

   VoxetProjectHandle *voxetProject = nullptr;

   if (voxetFileName == "")
   {
      ofstream voxetProjectFileStream;

      char tmpVoxetFileName[256];

      snprintf (tmpVoxetFileName, sizeof (tmpVoxetFileName), "/tmp/voxetProjectFile%d", getpid ());

      voxetProjectFileStream.open (tmpVoxetFileName);
      if (!voxetProjectFileStream.is_open ())
      {
         LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open temporary file " << tmpVoxetFileName << ", aborting ....";
         return -1;
      }

      createVoxetProjectFile (*projectHandle, propertyManager, voxetProjectFileStream, snapshot, propertyList);
      voxetProjectFileStream.close ();

      voxetProject = new VoxetProjectHandle (tmpVoxetFileName, *projectHandle);
      unlink (tmpVoxetFileName);
   }
   else
   {
      voxetProject = new VoxetProjectHandle (voxetFileName, *projectHandle);
   }

   snapshot = projectHandle->findSnapshot (voxetProject->getSnapshotTime ());

   // timeStamp was already appended with "_" in its assignment to avoid a condition here on its existence
   string asciiFileName = "BPA2_" + outputFileName + timeStamp + ".vo";

   if(singlePropertyHeader)
   {
      // timeStamp was already appended with "_" in its assignment to avoid a condition here on its existence
      asciiFileName = "BPA2_" + outputFileName + "_all" + timeStamp + ".vo";
   }
   string binaryFileName = outputFileName;

   ofstream asciiOutputFile;

   asciiOutputFile.open (asciiFileName.c_str ());

   if (asciiOutputFile.fail ())
   {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open output file " << asciiFileName;
      return -1;
   }

   if (projectFileName.length () == 0)
   {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open project file " << projectFileName;
      return -1;
   }

   if (verbose)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << "Using snapshot " << setprecision (10) << snapshot->getTime ();
   }

   const DataAccess::Interface::Property *depthProperty = projectHandle->findProperty ("Depth");

   if (!depthProperty)
   {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not find the Depth property in the project file. "
                                                                  << "Are you sure the project file contains output data?";
      return -1;
   }

   int propertyCount = 1;

   const GridDescription & gridDescription = voxetProject->getGridDescription ();

   VoxetCalculator vc (*projectHandle, propertyManager, voxetProject->getGridDescription (),propertyNullValueReplaceLookup);

   if (useBasement && verbose) LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << "Using basement";

   vc.setDepthProperty (depthProperty);


   // Array used to store interpolated values, this will then be output to file.
   VoxetPropertyGrid interpolatedProperty (voxetProject->getGridDescription ());

   asciiOutputFile.flags (ios::fixed);

   VoxetUtils::writeVOheader(asciiOutputFile, gridDescription, outputFileName);

   std::string propertyName;

   CauldronPropertyList::iterator cauldronPropIter;
   for (cauldronPropIter = voxetProject->cauldronPropertyBegin (); cauldronPropIter != voxetProject->cauldronPropertyEnd (); ++cauldronPropIter)
   {
      const DataAccess::Interface::Property *property = (*cauldronPropIter)->getProperty ();
      if (!property)
      {
         LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Unknown property: " << (*cauldronPropIter)->getCauldronName ();
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

      const PropertyValueList *propertyValueListAvailable = projectHandle->getPropertyValues (FORMATION, property, snapshot, nullptr, nullptr, nullptr, VOLUME);
      unsigned int propertiesSize = propertyValueListAvailable->size ();
      delete propertyValueListAvailable;
      if (propertiesSize == 0)
      {
         continue;
      }
#endif

      vc.useBasement() = useBasement;
      // Check if the property is computable at basement
      if (useBasement)
      {
          vc.useBasement() = VoxetUtils::isBasementProperty(property->getName());
      }

      AbstractDerivedProperties::FormationPropertyList propertyValueList ( propertyManager.getFormationProperties ( property, snapshot, vc.useBasement() ));

      // Could just ask if property is computable.
      unsigned int size = propertyValueList.size ();

      if (size == 0)
      {
         continue;
      }

      if (verbose)
      {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << " Adding cauldron property: " << property->getName ();
      }

      if ((*cauldronPropIter)->getVoxetOutput ())
      {

         vc.addProperty (property);

         if (vc.computeInterpolators (snapshot, verbose) == -1)
         {
            LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Are there any results in the project? ";
            return -1;
         }

         // timeStamp was already appended with "_" in its assignment to avoid a condition here on its existence
         std::string propertyFileName = "BPA2_" + binaryFileName + "_" + (*cauldronPropIter)->getCauldronName () + timeStamp + "@@";

         VoxetUtils::writeVOproperty(asciiOutputFile, propertyCount, *cauldronPropIter, propertyFileName, vc.getNullValue(property) );

         vc.computeProperty (*cauldronPropIter, interpolatedProperty, verbose);
         VoxetUtils::correctEndian (interpolatedProperty);
         VoxetUtils::write (propertyFileName, interpolatedProperty);
         ++propertyCount;

         if ( verbose )
         {
            LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << " deleting interpolators for property: " << property->getName ();
         }

         if(singlePropertyHeader)
         {
            std::ofstream asciiHeaderOutputFile;
            std::string asciiHeaderFileName = "BPA2_" + outputFileName + "_" + (*cauldronPropIter)->getCauldronName () + timeStamp +".vo";

            asciiHeaderOutputFile.open (asciiHeaderFileName.c_str ());

            if (asciiHeaderOutputFile.fail ())
            {
               LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open output file " << asciiHeaderFileName;
               return -1;
            }
            asciiHeaderOutputFile.flags (std::ios::fixed);

            VoxetUtils::writeVOheader(asciiHeaderOutputFile, gridDescription, outputFileName);

            VoxetUtils::writeVOproperty(asciiHeaderOutputFile, 1, *cauldronPropIter, propertyFileName, vc.getNullValue(property) );

            VoxetUtils::writeVOtail(asciiHeaderOutputFile);

            asciiHeaderOutputFile.close ();
         }
         vc.deleteProperty (property);
      }
   }

   VoxetUtils::writeVOtail(asciiOutputFile);
   asciiOutputFile.close ();

   if (debug)
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Project closed";

   return 0;
}


void createVoxetProjectFile(const DataAccess::Interface::ProjectHandle& cauldronProject,
    DerivedProperties::DerivedPropertyManager& propertyManager,
    ostream& outputStream, const DataAccess::Interface::Snapshot* snapshot, const std::vector<string>& propertyList)
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
       "Depth ",
       "Pressure", "Pressure: Overpressure", "Pressure: Hydrostatic", "Pressure: Lithostatic",
       "Temperature", "Vr",
       "Ves", "Max Ves",
       "Porosity", "Permeability", "Bulk Density",
       "Velocity", "Two Way Time",
       ""
    };

    const database::DataSchema* voxetSchema = database::createVoxetSchema();
    database::Database* database = database::Database::CreateFromSchema(*voxetSchema);
    database::Table* table;
    database::Record* record;
    const DataAccess::Interface::Grid* grid;

    //------------------------------------------------------------//

    table = database->getTable("CauldronPropertyIoTbl");
    for (int p = 0; strlen(propertyNames[p]) != 0; ++p)
    {
        const DataAccess::Interface::Property* property = cauldronProject.findProperty(propertyNames[p]);
        if (property)
        {
            if (propertyList.empty())
            {
                record = table->createRecord();
                database::setCauldronPropertyName(record, propertyNames[p]);
                database::setVoxetPropertyName(record, outputPropertyNames[p]);
                database::setOutputPropertyUnits(record, units[p]);
                database::setConversionFactor(record, conversions[p]);
                database::setVoxetOutput(record, 1);
            }
            else
            {
                auto it = std::find(propertyList.begin(), propertyList.end(), propertyNames[p]);
                if (it != propertyList.end())
                {
                    record = table->createRecord();
                    database::setCauldronPropertyName(record, propertyNames[p]);
                    database::setVoxetPropertyName(record, outputPropertyNames[p]);
                    database::setOutputPropertyUnits(record, units[p]);
                    database::setConversionFactor(record, conversions[p]);
                    database::setVoxetOutput(record, 1);
                }
            }
        }
    }

    //------------------------------------------------------------//

    table = database->getTable("SnapshotTimeIoTbl");

    record = table->createRecord();
    database::setSnapshotTime(record, snapshot->getTime());

    //------------------------------------------------------------//

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
    int minI = bottomDepthGrid->firstI(0);
    int maxI = bottomDepthGrid->lastI(0);
    int minJ = bottomDepthGrid->firstJ(0);
    int maxJ = bottomDepthGrid->lastJ(0);

    // Computing the maximum value of depth for the bottom formation grid
    double minDepthValInGrid = std::numeric_limits< double >::max();
    double maxDepthValInGrid = -std::numeric_limits< double >::max();

    for (int i = minI; i <= maxI; ++i)
    {
        for (int j = minJ; j <= maxJ; ++j)
        {
            double value = bottomDepthPropertyValue->get(i, j);
            // Ignoring Undefined Value = 99999 at nodes
            if ((value != 99999) && (value > maxDepthValInGrid))
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

    minI = topDepthGrid->firstI(0);
    maxI = topDepthGrid->lastI(0);
    minJ = topDepthGrid->firstJ(0);
    maxJ = topDepthGrid->lastJ(0);

    // Computing the minimum value of depth for the basin
    for (int i = minI; i <= maxI; ++i)
    {
        for (int j = minJ; j <= maxJ; ++j)
        {
            double value = topDepthPropertyValue->get(i, j);
            // Ignoring Undefined Value = 99999 at nodes
            if ((value != 99999) && (value < minDepthValInGrid))
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
    database::setVoxetOriginX(record, VoxetUtils::selectDefined(MAXDOUBLE, originX, grid->minI()));
    database::setVoxetOriginY(record, VoxetUtils::selectDefined(MAXDOUBLE, originY, grid->minJ()));
    database::setVoxetOriginZ(record, VoxetUtils::selectDefined(MAXDOUBLE, originZ, VoxetUtils::roundoff((minBasinDepth - 100),3)));
    database::setVoxetDeltaX(record, VoxetUtils::selectDefined(MAXDOUBLE, deltaX, grid->deltaI()));
    database::setVoxetDeltaY(record, VoxetUtils::selectDefined(MAXDOUBLE, deltaY, grid->deltaJ()));
    database::setVoxetDeltaZ(record, VoxetUtils::selectDefined(MAXDOUBLE, deltaZ, deltaK));
    database::setNumberOfVoxetNodesX(record, (int)VoxetUtils::selectDefined(MAXDOUBLE, countX, (double)grid->numI()));
    database::setNumberOfVoxetNodesY(record, (int)VoxetUtils::selectDefined(MAXDOUBLE, countY, (double)grid->numJ()));
    database::setNumberOfVoxetNodesZ(record, (int)VoxetUtils::selectDefined(MAXDOUBLE, countZ, ((maxBasinDepth - minBasinDepth) / deltaK) + 3));


    // The below piece of code was used to compute the total depth of the basin in BPA-legacy scenarios.
    // With updations in Cauldron libraries for BPA-2 scenarios, it is replaced with the code above for compatablity with BPA-2 scenarios
#if 0
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

#endif

    //------------------------------------------------------------//

    // Now write the stream to stdout.
    database->saveToStream(outputStream);
}
