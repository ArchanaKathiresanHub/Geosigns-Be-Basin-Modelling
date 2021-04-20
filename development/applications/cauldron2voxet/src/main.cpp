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
		<< "                  [-create-spec <spec-file>]" << endl
		<< "                  [-nullvaluereplace <PropertyName,Value> [<PropertyName,Value>] [...]]" << endl
		<< "                  [-nobasement]" << endl
		<< "                  [-propertyHeader]" << endl
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
		<< "    -create-spec          Write a standard spec file into the specified file name," << endl
		<< "                          the cauldron project file must also be specified." << endl
		<< "    -nullvaluereplace     Replace null values of the property by a given value." << endl
		<< "    -nobasement           Ignore basement layers." << endl
		<< "    -singlepropertyheader Writes one header file for each property. (additional to the multiple property header-file)" << endl
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

         splitString (c_origins, ',', c_originX, c_originY, c_originZ);

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

          splitString (c_deltas, ',', c_deltaX, c_deltaY, c_deltaZ);

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

          splitString (c_counts, ',', c_countX, c_countY, c_countZ);

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
              splitString (c_nullValueReplaceOption,',',nullValueReplaceName,nullValueReplaceValue,tmp);
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

   const Interface::SimulationDetails* simulationDetails = projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );
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


   const Snapshot *snapshot = projectHandle->findSnapshot (snapshotTime);

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

      createVoxetProjectFile (*projectHandle, propertyManager, voxetProjectFileStream, snapshot);
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

      createVoxetProjectFile (*projectHandle, propertyManager, voxetProjectFileStream, snapshot);
      voxetProjectFileStream.close ();

      voxetProject = new VoxetProjectHandle (tmpVoxetFileName, *projectHandle);
      unlink (tmpVoxetFileName);
   }
   else
   {
      voxetProject = new VoxetProjectHandle (voxetFileName, *projectHandle);
   }

   snapshot = projectHandle->findSnapshot (voxetProject->getSnapshotTime ());

   string asciiFileName = outputFileName + ".vo";
   if(singlePropertyHeader)
   {
      asciiFileName = outputFileName + "_all.vo";
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

   const Property *depthProperty = projectHandle->findProperty ("Depth");

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

   writeVOheader(asciiOutputFile, gridDescription, outputFileName);

   std::string propertyName;

   CauldronPropertyList::iterator cauldronPropIter;
   for (cauldronPropIter = voxetProject->cauldronPropertyBegin (); cauldronPropIter != voxetProject->cauldronPropertyEnd (); ++cauldronPropIter)
   {
      const Property *property = (*cauldronPropIter)->getProperty ();
      if (!property)
      {
         LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Unknown property: " << (*cauldronPropIter)->getCauldronName ();
         continue;
      }
      propertyName = property->getName();
      // Do not calculate the TwoWayTime after the decompaction run as it throws exception unlike other properties when attempted to compute
      // as the property can not be computed after just the decompaction run
      if (simulationMode_fastCauldron == "HydrostaticDecompaction" && (propertyName == "TwoWayTime")) continue;


#if 0
      // Check if the property has values (only for 2016.11 release. Remove this check to enable DerivedProperty calculation) -- removed using #if #endif
      const PropertyValueList *propertyValueListAvailable = projectHandle->getPropertyValues (FORMATION, property, snapshot, nullptr, nullptr, nullptr, VOLUME);
      unsigned int propertiesSize = propertyValueListAvailable->size ();
      delete propertyValueListAvailable;
      if (propertiesSize == 0)
      {
         continue;
      }
#endif

      vc.useBasement() = useBasement;
      if (useBasement)
      {
          vc.useBasement() = isBasementProperty(property->getName());
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

         std::string propertyFileName = binaryFileName + "_" + (*cauldronPropIter)->getCauldronName () + "@@";

         writeVOproperty(asciiOutputFile, propertyCount, *cauldronPropIter, propertyFileName, vc.getNullValue(property) );

         vc.computeProperty (*cauldronPropIter, interpolatedProperty, verbose);
         correctEndian (interpolatedProperty);
         write (propertyFileName, interpolatedProperty);
         ++propertyCount;

         if ( verbose )
         {
            LogHandler(LogHandler::INFO_SEVERITY, LogHandler::DEFAULT) << " deleting interpolators for property: " << property->getName ();
         }

         if(singlePropertyHeader)
         {
            std::ofstream asciiHeaderOutputFile;
            std::string asciiHeaderFileName=outputFileName + "_" + (*cauldronPropIter)->getCauldronName ()+".vo";

            asciiHeaderOutputFile.open (asciiHeaderFileName.c_str ());

            if (asciiHeaderOutputFile.fail ())
            {
               LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Could not open output file " << asciiHeaderFileName;
               return -1;
            }
            asciiHeaderOutputFile.flags (std::ios::fixed);

            writeVOheader(asciiHeaderOutputFile, gridDescription, outputFileName);

            writeVOproperty(asciiHeaderOutputFile, 1, *cauldronPropIter, propertyFileName, vc.getNullValue(property) );

            writeVOtail(asciiHeaderOutputFile);

            asciiHeaderOutputFile.close ();
         }
         vc.deleteProperty (property);
      }
   }

   writeVOtail(asciiOutputFile);
   asciiOutputFile.close ();

   if (debug)
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << "Project closed";

   return 0;
}
