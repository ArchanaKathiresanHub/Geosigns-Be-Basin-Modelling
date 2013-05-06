#include <iostream>
#include <string>
#include <cassert>
#include <sstream>

#include "parameterspaceexplorationtoolfuncs.h"
#include "parameterspaceexplorationtool.h"
#include "BasementProperty.h"
#include "CrustalThinningProperty.h"
#include "RuntimeConfiguration.h"
#include "DatadrillerProperty.h"
#include "experiment.h"
#include "project.h"
#include "InputParameterDataRow.h"

using namespace std;


int main () 
{
   const string fileName = "parameter-space-exploration-tool.config";
   database::DataSchema* schema = database::createParameterSpaceExplorationTool ();

   database::Database* database = database::Database::CreateFromFile ( fileName, *schema );

   database::Table* configuration = database->getTable("RuntimeConfiguration");
   database::Table* basementPropertyTable = database->getTable("BasementProperty");
   database::Table* crustalThinningPropertyTable = database->getTable("CrustalThinningProperty");
   database::Table* datadrillerPropertyTable = database->getTable("DatadrillerProperty");

   std::vector< RuntimeConfiguration > info;
   for (int i = 0; i < configuration->size (); ++i ) 
   {
      database::Record* record_info = configuration->getRecord ( i );

      std::string templateProjectFile = database::getProjectTemplatePath ( record_info );
      std::string outputDirectoryAddress = database::getOutputDirectory ( record_info );
      std::string outputFileNamePrefix = database::getOutputFileName ( record_info );

      info.push_back( RuntimeConfiguration( templateProjectFile, outputDirectoryAddress, outputFileNamePrefix ));
   }

   std::vector< Property* > params;

   for (int i = 0; i < basementPropertyTable->size (); ++i ) 
   {
      database::Record* record = basementPropertyTable->getRecord ( i );

      std::string simpleParameter = database::getSimpleParameter ( record );
      double startValue = database::getStartValue ( record );
      double endValue = database::getEndValue ( record );
      double stepValue = database::getStepValue ( record );
    
      params.push_back( new BasementProperty( simpleParameter, startValue, endValue, stepValue ));
   }


//  if (crustalThinningPropertyTable->size () != 4 )
//   {
//    cerr << "Error! The Crustal thinning table should contain 4 lines for the 4 following parameters: InitialCrustalThinningTime, CrustalThinningDuration, //InitialCrustalThinningThickness, CrustalThinningRatio " << endl;
//    return 0;
//   }
// nlsay3 28/03
   vector<InputParameterDataRow> RowVector;
   for (int i = 0; i < crustalThinningPropertyTable->size (); ++i ) 
   {
      database::Record* recordCrustalThin = crustalThinningPropertyTable->getRecord ( i );

      std::string simpleParameter = database::getSimpleParameter ( recordCrustalThin );
      double startValue = database::getStartValue ( recordCrustalThin );
      double endValue = database::getEndValue ( recordCrustalThin );
      double stepValue = database::getStepValue ( recordCrustalThin );

      RowVector.push_back( InputParameterDataRow(simpleParameter, startValue, endValue, stepValue) );
   }

   if (crustalThinningPropertyTable->size() > 0)
   {
      params.push_back( new CrustalThinningProperty( RowVector ) );
   }


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

  Experiment allProjects(params, drillerInfo, info[0]);

  allProjects.runProjectSet(allProjects.createProjectsSet());

  allProjects.readExperimentResults();
}

