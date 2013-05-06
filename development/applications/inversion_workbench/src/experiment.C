#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "experiment.h"

#include "BasementProperty.h"
#include "RuntimeConfiguration.h"
#include "DatadrillerProperty.h"
#include "case.h"

#include <unistd.h>

Experiment :: Experiment( std::vector< Property *> params, std::vector<DatadrillerProperty> & datadrillerDefinitions, RuntimeConfiguration & dataInfo)
   : m_cases(1, Case() )
   , m_experimentInfo(dataInfo)
{
   //m_cases.push_back( Case() );
   Experiment::sample( params, m_cases);
   Experiment::defineDatamining(datadrillerDefinitions , m_cases);
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
         lastParameterDefinition->createParameter(project, value);

/*    Case project = allProjects[i];
      pt_param = Parameter( lastParameterDefinition->getName(), value);
      project.addParameter( pt_param );*/

         newOutput.push_back( project );
      }
   }

   allProjects.swap( newOutput );
}


void Experiment::defineDatamining( std::vector<DatadrillerProperty> & datadrillerDefinitions, std::vector< Case > & allProjects )
{
   if (datadrillerDefinitions.empty())
     return;

   DatadrillerProperty lastDatadrillerVariableDefinition = datadrillerDefinitions.back();
   datadrillerDefinitions.pop_back(); 

   defineDatamining(datadrillerDefinitions, allProjects );

   std::vector< Case > newOutput;

   for (unsigned i = 0; i < allProjects.size(); ++i)
   {
      Case project = allProjects[i];
      project.addVariableToDrill( DatadrillerProperty( lastDatadrillerVariableDefinition.getName() ) );
      project.defineLocationToDrill(lastDatadrillerVariableDefinition);
      newOutput.push_back( project );
   }

   allProjects.swap( newOutput );
}



std::vector<std::string> Experiment :: createProjectsSet()
{
   std::vector<std::string> projectsList;
   std::string inputProject = m_experimentInfo.getTemplateProjectFile();
   std::string outputProjectWithoutExtension = m_experimentInfo.getOutputFileNamePrefix();
   std::string::size_type dotPos = outputProjectWithoutExtension.rfind (".project3d");
   if (dotPos != std::string::npos)
   {
      outputProjectWithoutExtension.erase(dotPos, std::string::npos);
   }

   std::ostringstream directory;
   directory << m_experimentInfo.getOutputDirectoryAddress() << "./";

   for (unsigned i = 0; i < m_cases.size(); ++i)
   {
      std::ostringstream outputProject;
      outputProject 
         << outputProjectWithoutExtension
         << "_" << i+1 
         << ".project3d";

      m_cases[i].createProjectFile(inputProject, directory.str() + outputProject.str());
      m_cases[i].setProjectFile(directory.str() + outputProject.str());
      m_cases[i].setResultsFile(directory.str() + "Datadriller_" + outputProject.str());

      projectsList.push_back(directory.str() + outputProject.str());
   }

  return projectsList;
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


void Experiment :: readExperimentResults()
{
   for (unsigned i=0; i < m_cases.size(); ++i)
   {
      m_cases[i].readProjectFile();
      m_cases[i].displayResults();
   }
}


void Experiment::readExperimentCases()
{
   for (int i=0; i < m_cases.size(); ++i)
   {
      m_cases[i].readProjectFile();
      m_cases[i].displayResults();
   }
}


void Experiment::displayCases() const
{
   for (int i=0; i < m_cases.size(); ++i)
   {
      m_cases[i].displayParameters();
   }
}
