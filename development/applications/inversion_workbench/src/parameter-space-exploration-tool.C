#include <iostream>
#include <string>
#include <cassert>
#include <sstream>

#include "parameterspaceexplorationtoolfuncs.h"
#include "parameterspaceexplorationtool.h"
#include "BasementProperty.h"
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
  database::Table* BasementPropertyTable = database->getTable("BasementProperty");
  database::Table* CrustalThinningPropertyTable = database->getTable("CrustalThinningProperty");
  database::Table* DatadrillerPropertyTable = database->getTable("DatadrillerProperty");

  std::vector< RuntimeConfiguration > info;
  for (int i = 0; i < configuration->size (); ++i ) {
    database::Record* record_info = configuration->getRecord ( i );

    std::string templateProjectFile = database::getProjectTemplatePath ( record_info );
    std::string OutputDirectoryAddress = database::getOutputDirectory ( record_info );
    std::string OutputFileNamePrefix = database::getOutputFileName ( record_info );

    cout << templateProjectFile << " " << OutputDirectoryAddress << " " << OutputFileNamePrefix << endl;
    info.push_back( RuntimeConfiguration( templateProjectFile, OutputDirectoryAddress, OutputFileNamePrefix ));
  }

  std::vector< Property* > params;

  for (int i = 0; i < BasementPropertyTable->size (); ++i ) {
    database::Record* record = BasementPropertyTable->getRecord ( i );

    std::string simpleParameter = database::getSimpleParameter ( record );
    double startValue = database::getStartValue ( record );
    double endValue = database::getEndValue ( record );
    double stepValue = database::getStepValue ( record );
    
//    cout << simpleParameter << " " << startValue << " " << endValue << " " << stepValue << endl;
    params.push_back( new BasementProperty( simpleParameter, startValue, endValue, stepValue ));
  }

    for (int i = 0; i < params.size (); ++i ) {
	cout << params[i]->getName() << endl;

}

//  if (CrustalThinningPropertyTable->size () != 4 )
//   {
//    cerr << "Error! The Crustal thinning table should contain 4 lines for the 4 following parameters: InitialCrustalThinningTime, CrustalThinningDuration, //InitialCrustalThinningThickness, CrustalThinningRatio " << endl;
//    return 0;
//   }
// nlsay3 28/03
  cout << "CrustalThinningPropertyTable->size: " << CrustalThinningPropertyTable->size() << endl;
  vector<InputParameterDataRow> RowVector;
  for (int i = 0; i < CrustalThinningPropertyTable->size (); ++i ) {
    database::Record* record_crustal_thin = CrustalThinningPropertyTable->getRecord ( i );

    std::string simpleParameter = database::getSimpleParameter ( record_crustal_thin );
    double startValue = database::getStartValue ( record_crustal_thin );
    double endValue = database::getEndValue ( record_crustal_thin );
    double stepValue = database::getStepValue ( record_crustal_thin );

    RowVector.push_back( InputParameterDataRow(simpleParameter, startValue, endValue, stepValue) );

//    cout << simpleParameter << " " << startValue << " " << endValue << " " << stepValue << endl;
  }

  if (CrustalThinningPropertyTable->size() > 0)
  {
    params.push_back( new CrustalThinningProperty( RowVector ) );
  }


  for (int i = 0; i < params.size(); ++i ) {
	cout << params[i]->getName() << endl;
	cout << params[i]->getStart() << "  " << params[i]->getEnd() << "  " << params[i]->getStep() << endl;

}


  std::vector< DatadrillerProperty > driller_info;
  for (int i = 0; i < DatadrillerPropertyTable->size (); ++i ) {
    database::Record* record_driller_info = DatadrillerPropertyTable->getRecord ( i );

    std::string RetrievedVariable = database::getRetrievedVariable ( record_driller_info );
    double SnapshotTime = database::getSnapshotTime ( record_driller_info );
    double PositionX = database::getPositionX ( record_driller_info );
    double PositionY = database::getPositionY ( record_driller_info );
    double PositionBegZ = database::getPositionBegZ ( record_driller_info );
    double PositionEndZ = database::getPositionEndZ ( record_driller_info );
    double StepZ = database::getStepZ ( record_driller_info );
    
//    cout << RetrievedVariable << " " << SnapshotTime << " " << PositionX << " " << PositionY << endl;
    driller_info.push_back( DatadrillerProperty( RetrievedVariable, SnapshotTime, PositionX, PositionY, PositionBegZ, PositionEndZ, StepZ ));
  }

  std::cout << "Before Experiment constructor called..." << std::endl;

  Experiment allProjects(params, driller_info, info[0]);

  cout << "-------------------------" << endl;
  allProjects.display_Cases();

  allProjects.runProjectSet(allProjects.create_projects_set());

  allProjects.ReadExperimentResults();


/*  readProjectFile( const std::string & project, const std::string & propertyName, double snapshotTime, double x, double y,
    const std::vector<double> & zs, std::vector<double> & results );*/
  
}










//  cout << allProjects.m_cases[0].m_values[0].m_value << endl;;

/*  cout << "All combinations are: " << endl;
  for (unsigned i = 0; i < allProjects.m_cases.size(); ++i)
  {
    for (unsigned j = 0; j < allProjects[i].size(); ++j)
    {
      if (j > 0)
        cout << "| ";

      allProjects[i][j].print() ;
    }
    cout << endl;
  }

  return 0;
*/


/*  cout << "All combinations are: " << endl;
  for (unsigned i = 0; i < allProjects.size(); ++i)
  {
    for (unsigned j = 0; j < allProjects[i].size(); ++j)
    {
      if (j > 0)
        cout << "| ";

      allProjects[i][j].print() ;
    }
    cout << endl;
  }

  return 0;

}
*/
