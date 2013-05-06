#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "BasementProperty.h"
#include "RuntimeConfiguration.h"
#include "experiment.h"
#include <unistd.h>

Experiment :: Experiment( std::vector< Property *> params, std::vector<DatadrillerProperty> & DatadrillerDefinitions, RuntimeConfiguration & datainfo)
  : m_cases(1, Case() )
  , m_experiment_info(datainfo)
{
//m_cases.push_back( Case() );
Experiment::sample( params, m_cases);
Experiment::define_datamining(DatadrillerDefinitions , m_cases);
}

void Experiment::sample( std::vector<Property *> parameterDefinitions, std::vector< Case > & allProjects )
{

  if (parameterDefinitions.empty())
    return;

  Property* lastParameterDefinition = parameterDefinitions.back();
  parameterDefinitions.pop_back(); 

  sample(parameterDefinitions, allProjects );

  std::vector< Case > newOutput;

  for (unsigned i = 0; i < allProjects.size(); ++i)
  {
    for (double value = lastParameterDefinition->getStart() ; value <= lastParameterDefinition->getEnd(); value += lastParameterDefinition->getStep() )  
    {

      Case project = allProjects[i];
      lastParameterDefinition->CreateParameter(project, value);

/*    Case project = allProjects[i];
      pt_param = Parameter( lastParameterDefinition->getName(), value);
      project.addParameter( pt_param );*/

      newOutput.push_back( project );
    }
  }

  allProjects.swap( newOutput );
}



void Experiment::define_datamining( std::vector<DatadrillerProperty> & DatadrillerDefinitions, std::vector< Case > & allProjects )
{
  if (DatadrillerDefinitions.empty())
    return;

  DatadrillerProperty lastDatadrillerVariableDefinition = DatadrillerDefinitions.back();
  DatadrillerDefinitions.pop_back(); 

  define_datamining(DatadrillerDefinitions, allProjects );

  std::vector< Case > newOutput;

  for (unsigned i = 0; i < allProjects.size(); ++i)
  {
    Case project = allProjects[i];
    project.addVariableToDrill( DatadrillerProperty( lastDatadrillerVariableDefinition.getName() ) );
    project.Define_location_to_drill(lastDatadrillerVariableDefinition);
    newOutput.push_back( project );
  }

  allProjects.swap( newOutput );

}



std::vector<std::string> Experiment :: create_projects_set()
{
  std::vector<std::string> projects_list;
  std::string inputProject = m_experiment_info.getTemplateProjectFile();
  std::string outputProjectWithoutExtension = m_experiment_info.getOutputFileNamePrefix();
  std::string::size_type dotPos = outputProjectWithoutExtension.rfind (".project3d");
  if (dotPos != std::string::npos)
  {
    outputProjectWithoutExtension.erase(dotPos, std::string::npos);
  }

  std::ostringstream directory;
  directory << m_experiment_info.getOutputDirectoryAddress() << "./";

  for (unsigned i = 0; i < m_cases.size(); ++i)
  {
    std::ostringstream outputProject;
    outputProject 
      << outputProjectWithoutExtension
      << "_" << i+1 
      << ".project3d";

    m_cases[i].create_project_file(inputProject, directory.str() + outputProject.str());
    m_cases[i].set_ProjectFile(directory.str() + outputProject.str());
    m_cases[i].set_ResultsFile(directory.str() + "Datadriller_" + outputProject.str());

    projects_list.push_back(directory.str() + outputProject.str());

  }

  return projects_list;
}


/*  void Experiment :: run_projects_set(std::string File_List)
{
  std::string command="./TEST_FOLDER/cauldron-datadriller-parallel.sh ";
  command=command+File_List;
  system(command.c_str());

}*/

void Experiment :: runProjectSet( const std::vector< std::string > & fileList)
{
  const std::string version = "2012.1008";
  const std::string fastcauldronPath = "fastcauldron";
  const std::string runtimeParams = "-temperature";

//  #pragma omp parallel for
  for (unsigned i = 0; i < fileList.size(); ++i)
  {
    std::ostringstream command;
    command << fastcauldronPath 
            << " -v" << version
            << " -project " << fileList[i]
            << ' ' << runtimeParams;
    
  system( command.str().c_str() );
  }
}


void Experiment :: ReadExperimentResults()
{
  for (unsigned i=0; i < m_cases.size(); ++i)
  {
    m_cases[i].readProjectFile();
    m_cases[i].display_results();
  }
}


void Experiment::ReadExperimentCases()
{
  for (int i=0; i < m_cases.size(); ++i)
  {
    m_cases[i].readProjectFile();
    m_cases[i].display_results();
  }

}


void Experiment::display_Cases() const
{
  for (int i=0; i < m_cases.size(); ++i)
  {
    m_cases[i].display_Parameters();
  }
}
