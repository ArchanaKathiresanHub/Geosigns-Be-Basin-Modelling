#include <iostream>
#include <string>
#include <cassert>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "parameterspaceexplorationtoolfuncs.h"
#include "parameterspaceexplorationtool.h"
#include "BasementProperty.h"
#include "CrustalThinningProperty.h"
#include "RuntimeConfiguration.h"
#include "DatadrillerProperty.h"
#include "experiment.h"
#include "project.h"


int main(int argc, char ** argv ) 
{
   // parse command line parameters
   std::string fileName = "parameter-space-exploration-tool.config";
   if (argc > 1)
      fileName = argv[1];

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

   database::Table* configuration = database->getTable("RuntimeConfiguration");
   database::Table* basementPropertyTable = database->getTable("BasementProperty");
   database::Table* crustalThinningPropertyTable = database->getTable("CrustalThinningProperty");
   database::Table* datadrillerPropertyTable = database->getTable("DatadrillerProperty");

   assert( configuration );
   assert( basementPropertyTable );
   assert( crustalThinningPropertyTable );
   assert( datadrillerPropertyTable );

   // read runtime configuration parameters
   std::vector< RuntimeConfiguration > info;
   for (int i = 0; i < configuration->size (); ++i ) 
   {
      database::Record* recordInfo = configuration->getRecord ( i );
      assert( recordInfo );

      std::string templateProjectFile = database::getProjectTemplatePath ( recordInfo );
      std::string outputDirectoryAddress = database::getOutputDirectory ( recordInfo );
      std::string outputFileName = database::getOutputFileName ( recordInfo );

      // if the outputFileName has an extension ".project3d", remove it, because we only want the prefix.
      std::string::size_type dotPos = outputFileName.rfind (".project3d");
      if (dotPos != std::string::npos)
      {
         outputFileName.erase(dotPos, std::string::npos);
      }

      // Add it to the list
      info.push_back( RuntimeConfiguration( templateProjectFile, outputDirectoryAddress, outputFileName ));
   }

   if (info.empty())
   {
      std::cerr << "Error: no runtime configuration present in experiment "
         << "set-up '" << fileName << "'" << std::endl;
      return EXIT_FAILURE;
   }

   // Read basement properties
   std::vector< boost::shared_ptr<Property> > params;
   for (int i = 0; i < basementPropertyTable->size (); ++i ) 
   {
      database::Record* record = basementPropertyTable->getRecord ( i );

      std::string simpleParameter = database::getSimpleParameter ( record );
      double startValue = database::getStartValue ( record );
      double endValue = database::getEndValue ( record );
      double stepValue = database::getStepValue ( record );
    
      params.push_back( boost::shared_ptr<Property>( 
               new BasementProperty( simpleParameter, startValue, endValue, stepValue )
               ) );
   }


   { // Read crustal thinning properties
      if (crustalThinningPropertyTable->size () % 4 != 0 || crustalThinningPropertyTable->size() > 4 )
      {
         std::cerr << "Error! The Crustal thinning table should contain none or exactly 4 records for the 4 following parameters: InitialCrustalThinningTime, CrustalThinningDuration, InitialCrustalThinningThickness, CrustalThinningRatio " << std::endl;
         return EXIT_FAILURE;
      }

      std::string crustalThinningSimpleParamNames[] 
         = { "InitialCrustalThinningTime", "CrustalThinningDuration", 
            "InitialCrustalThinningThickness", "CrustalThinningRatio" };

      std::vector< ScalarRange > crustalThinningRanges;
      for (int i = 0; i < crustalThinningPropertyTable->size (); ++i ) 
      {
         database::Record* recordCrustalThin = crustalThinningPropertyTable->getRecord ( i );

         if ( crustalThinningSimpleParamNames[i % 4 ] != database::getSimpleParameter ( recordCrustalThin ))
         {
            std::cerr << "Error in experiment set-up '" << fileName << "' in table 'CrustalThinningProperty': "
                << "expected '" << crustalThinningSimpleParamNames[i % 4] << "' instead of '"
                << database::getSimpleParameter ( recordCrustalThin ) << "'" << std::endl;
            return EXIT_FAILURE;
         }

         crustalThinningRanges.push_back( ScalarRange( database::getStartValue ( recordCrustalThin ),
                  database::getEndValue ( recordCrustalThin ),
                  database::getStepValue ( recordCrustalThin )
                  ));

         if (crustalThinningRanges.size() == 4)
         {
            params.push_back( boost::shared_ptr<Property>( 
                     new CrustalThinningProperty( crustalThinningRanges[0]
                        , crustalThinningRanges[1], crustalThinningRanges[2], crustalThinningRanges[3]
                        )
                  ));
            crustalThinningRanges.clear();
         }
      }
   }

   // Read in the locations where the measurements should be taken
   std::vector< DatadrillerProperty > drillerInfo;
   for (int i = 0; i < datadrillerPropertyTable->size (); ++i ) 
   {
      database::Record* recordDrillerInfo = datadrillerPropertyTable->getRecord ( i );

      std::string retrievedVariable = database::getRetrievedVariable ( recordDrillerInfo );
      double snapshotTime = database::getSnapshotTime ( recordDrillerInfo );
      double positionX = database::getPositionX ( recordDrillerInfo );
      double positionY = database::getPositionY ( recordDrillerInfo );
      double positionBegZ = database::getPositionBegZ ( recordDrillerInfo );
      double positionEndZ = database::getPositionEndZ ( recordDrillerInfo );
      double stepZ = database::getStepZ ( recordDrillerInfo );
    
      drillerInfo.push_back( DatadrillerProperty( retrievedVariable, snapshotTime, positionX, positionY, positionBegZ, positionEndZ, stepZ ));
   }

   // Run the experiment
   Experiment experiment(params, drillerInfo, info[0]);
   experiment.createProjectsSet();
   experiment.runProjectSet();
   experiment.collectResults();

   return EXIT_SUCCESS;
}

