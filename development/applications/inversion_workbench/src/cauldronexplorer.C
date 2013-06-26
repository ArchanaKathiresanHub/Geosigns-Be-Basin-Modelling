#include <iostream>
#include <string>
#include <cassert>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include <boost/shared_ptr.hpp>

#include "parameterspaceexplorationtoolfuncs.h"
#include "parameterspaceexplorationtool.h"
#include "BasementProperty.h"
#include "CrustalThinningProperty.h"
#include "InitialCrustalThicknessProperty.h"
#include "UnconformityLithologyProperty.h"
#include "UnconformityProperty.h"
#include "RuntimeConfiguration.h"
#include "DatadrillerProperty.h"
#include "experiment.h"
#include "project.h"
#include "formattingexception.h"

const std::string DEFAULT_CONFIG_FILE = "cauldronexplorer.config";

static void showUsage (const char *argv0, const char * message = 0);

struct ConfigurationException : formattingexception :: BaseException< ConfigurationException > {};

void readBasementProperties( database::Database & database,
                             std::vector< boost::shared_ptr< Property > > & params)
{
   database::Table * table = database.getTable("BasementProperty");
   if (!table)
      return;
   
   for (int i = 0; i < table->size (); ++i ) 
   {
      database::Record* record = table->getRecord ( i );

      std::string simpleParameter = database::getSimpleParameter ( record );
      double startValue = database::getStartValue ( record );
      double endValue = database::getEndValue ( record );
      double stepValue = database::getStepValue ( record );
    
      params.push_back( boost::shared_ptr<Property>( 
               new BasementProperty( simpleParameter, startValue, endValue, stepValue )
               ) );
   }
}

void readUnconformityLithologies( database::Database & database,
                             std::vector< boost::shared_ptr< Property > > & params)
{
   database::Table * table = database.getTable("UnconformityLithology");
   if (!table)
      return;
   
   for (int i = 0; i < table->size (); ++i ) 
   {
      database::Record* record = table->getRecord ( i );

      std::string depoFormationName = database::getDepoFormationName ( record );

      std::string lithology1 = database::getLithology1 ( record );
      double percentage1 = database::getPercentage1 ( record );
    
      std::string lithology2 = database::getLithology2 ( record );
      double percentage2 = database::getPercentage2 ( record );

      std::string lithology3 = database::getLithology3 ( record );
      double percentage3 = database::getPercentage3 ( record );

      params.push_back( boost::shared_ptr<Property>( 
               new UnconformityLithologyProperty( depoFormationName, lithology1, percentage1, lithology2, percentage2, lithology3, percentage3 )
               ) );
   }
}

void readUnconformityProperties( database::Database & database,
                             std::vector< boost::shared_ptr< Property > > & params)
{
   database::Table * table = database.getTable("UnconformityProperty");
   if (!table)
      return;
   
   for (int i = 0; i < table->size (); ++i ) 
   {
      database::Record* record = table->getRecord ( i );

      std::string depoFormationName = database::getDepoFormationName ( record );

      std::string simpleParameter = database::getSimpleParameter ( record );
      double startValue = database::getStartValue ( record );
      double endValue = database::getEndValue ( record );
      double stepValue = database::getStepValue ( record );
    
      params.push_back( boost::shared_ptr<Property>( 
               new UnconformityProperty( depoFormationName, simpleParameter, startValue, endValue, stepValue )
               ) );
   }
}

void readCrustalThinningProperties( database::Database & database,
                                    std::vector< boost::shared_ptr< Property > > & params )
{
   database::Table * table = database.getTable ("CrustalThinningProperty");
   if (!table || table->size () == 0)
      return;

   double initialThicknessStart = 0.0;
   double initialThicknessEnd = 0.0;
   double initialThicknessStep = 0.0;

   if (table->getRecord (0) &&
	 database::getSimpleParameter (table->getRecord (0)) == "InitialCrustalThinningThickness")
   {
      database::Record * record = table->getRecord (0);
      initialThicknessStart = database::getStartValue (record);
      initialThicknessEnd = database::getEndValue (record);
      initialThicknessStep = database::getStepValue (record);
   }
   else
   {
      throw ConfigurationException () << "The CrustalThinningProperty table "
            "should be empty or its first record should be a "
	    "InitialCrustalThinningThickness record.";
   }

   params.push_back (boost::shared_ptr < Property > ( new InitialCrustalThicknessProperty (initialThicknessStart, initialThicknessEnd, initialThicknessStep)));

   std::string paramNames[] = { "InitialCrustalThinningTime", "CrustalThinningDuration", "CrustalThinningRatio" };

   std::vector < ScalarRange > ranges;
   for (int i = 1; i < table->size (); ++i)
   {
      database::Record * record = table->getRecord (i);

      if (paramNames[(i - 1) % 3] != database::getSimpleParameter (record))
         throw ConfigurationException ()
	    << "Expected in CrustalThinningProperty table on record " << i
	    << "a " << paramNames[(i - 1) % 3] << " property";

      ranges.push_back (ScalarRange (database::getStartValue (record),
	       database::getEndValue (record),
	       database::getStepValue (record)
	       ));

      if (ranges.size () == 3)
      {
         params.push_back (boost::shared_ptr < Property > (
		  new CrustalThinningProperty (ranges[0], ranges[1], ranges[2])
		  ));
         ranges.clear ();
      }
   }

   if (!ranges.empty ())
      throw ConfigurationException () << "The last " << ranges.size () <<
            " records in the CrustalThinningProperty table do no form a complete"
	    " thinning event range";
}

std::vector< DatadrillerProperty > readDatadrillerProperties( database::Database & database )
{
   database::Table* table = database.getTable("DatadrillerProperty");
   if (!table || table->size() == 0)
      throw ConfigurationException() << "There should be at least one entry "
         "in the DatadrillerProperty table";

   std::vector< DatadrillerProperty > result ;
   for (int i = 0; i < table->size (); ++i ) 
   {
      database::Record* record = table->getRecord ( i );

      std::string retrievedVariable = database::getRetrievedVariable ( record);
      double snapshotTime = database::getSnapshotTime ( record);
      double positionX = database::getPositionX ( record);
      double positionY = database::getPositionY ( record);
      double positionBegZ = database::getPositionBegZ ( record);
      double positionEndZ = database::getPositionEndZ ( record);
      double stepZ = database::getStepZ ( record);
    
      result.push_back( DatadrillerProperty( 
               retrievedVariable, 
               snapshotTime, 
               positionX,
               positionY,
               positionBegZ,
               positionEndZ,
               stepZ )
            );
   }
   return result;
}

RuntimeConfiguration readRuntimeConfiguration( database::Database & database)
{
   database::Table* configuration = database.getTable("RuntimeConfiguration");

   if (!configuration || configuration->size() != 1)
      throw ConfigurationException() << "There must be exactly one entry in "
         "the RuntimeConfiguration table";

   database::Record* recordInfo = configuration->getRecord ( 0 );
   assert( recordInfo );

   std::string templateProjectFile = database::getProjectTemplatePath ( recordInfo );
   std::string outputDirectoryAddress = database::getOutputDirectory ( recordInfo );
   std::string outputFileName = database::getOutputFileName ( recordInfo );
   std::string cauldronVersion = database::getCauldronVersion ( recordInfo );
   std::string fieldSeparator = database::getOutputTableFieldSeparator( recordInfo );
   int fieldWidth = database::getOutputTableFieldWidth( recordInfo );

   // if the outputFileName has an extension ".project3d", remove it, because we only want the prefix.
   std::string::size_type dotPos = outputFileName.rfind (".project3d");
   if (dotPos != std::string::npos)
   {
      outputFileName.erase(dotPos, std::string::npos);
   }

   // Init cauldronVersion with a default value, if it wasn't defined
   if (cauldronVersion.empty())
      cauldronVersion = "2012.1008";

   // Init fieldSeparator with a default value
   if (fieldSeparator.empty())
      fieldSeparator = ',';

   if (fieldSeparator.size() > 1)
      throw ConfigurationException() << "The OutputTableFieldSeparator can only consist of 1 character";


   // Add it to the list
   return RuntimeConfiguration( 
            templateProjectFile, 
            outputDirectoryAddress, 
            outputFileName, 
            cauldronVersion, 
            "-temperature",
            fieldSeparator[0],
            fieldWidth
            );
}


int main(int argc, char ** argv ) 
{
   const char * argv0 = strrchr (argv[0], '/');
   if (argv0 != 0)
      argv0++;
   else
      argv0 = argv[0];

   // parse command line parameters
   bool dryrun = false;
   std::string fileName = DEFAULT_CONFIG_FILE;
   std::ostream * verbose = 0;
   for (int arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-verbose", std::max((size_t) 5, strlen (argv[arg]))) == 0)
      {
         verbose = &std::cout;
      }
      else if (strncmp (argv[arg], "-dryrun", std::max((size_t) 2, strlen (argv[arg]))) == 0)
      {
	 dryrun = true;
      }
      else if (strncmp (argv[arg], "-help", std::max((size_t) 2, strlen (argv[arg]))) == 0)
      {
	 showUsage (argv0);
         return EXIT_SUCCESS;
      }
      else if (strncmp (argv[arg], "-config", std::max ((size_t) 2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage (argv0, "Parameter '-config' expects the name of the configuration file as an argument");
            return EXIT_FAILURE;
         }
	 fileName = argv[++arg];
      }
      else
      {
         fileName = argv[arg];
      }

   }

   // Read experiment set-up
   database::DataSchema* schema = database::createParameterSpaceExplorationTool ();
   if (!schema)
   {
      std::cerr << "Internal error: Could not create schema for experiment set-up file" << std::endl;
      return EXIT_FAILURE;
   }

   database::Database* database = database::Database::CreateFromFile ( fileName, *schema );
   if (!database)
   {
      std::cerr << "Could not read experiment set-up file '" << fileName << "'" << std::endl;
      return EXIT_FAILURE;
   }

   // Read properties
   std::vector< boost::shared_ptr<Property> > properties;
   readBasementProperties(*database, properties);
   readCrustalThinningProperties(*database, properties);
   readUnconformityLithologies(*database, properties);
   readUnconformityProperties(*database, properties);

   // Run the experiment
   Experiment experiment(properties, 
         readDatadrillerProperties(*database), 
         readRuntimeConfiguration(*database)
         );

   if (verbose)
      experiment.printScenarios(*verbose);

   experiment.createProjectsSet();

   if (!dryrun)
   {
      experiment.runProjectSet(verbose);
      experiment.collectResults();
   }

   return EXIT_SUCCESS;
}

void showUsage (const char * argv0, const char * message)
{
   std::cerr << std::endl;
   if (message)
   {
      std::cerr << argv0 << ": " << message << std::endl;
   }

   std::cerr << "Usage (Options may be abbreviated):\n "
         << std::setw(13) 
         << argv0         << "       [-config <file name>]\n"
         << "                        [-verbose]\n"
         << "                        [-dryrun]\n"
         << "                        [-help]\n"
         << "\n"                                               
         << "    -config <file name>  The configuration file to use. By default the file it searches for is '" 
                        << DEFAULT_CONFIG_FILE << "'.\n"
         << "    -verbose             Generate additional output on what is happening.\n"
         << "    -dryrun              Do not actually perform the simulations and the data collection.\n"
         << "    -help                Print this message and exit.\n"
         << std::endl;
}

