#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <memory>

#include "experiment.h"

#include "BasementProperty.h"
#include "RuntimeConfiguration.h"
#include "DatadrillerProperty.h"
#include "case.h"
#include "projectdependencies.h"
#include "formattingexception.h"
#include "system.h"

#include "Interface/ProjectHandle.h"

Experiment :: Experiment( const std::vector< boost::shared_ptr<Property> > & params, const std::vector<DatadrillerProperty> & datadrillerDefinitions, const RuntimeConfiguration & dataInfo)
   : m_cases(1, Case() )
   , m_probes(datadrillerDefinitions)
   , m_experimentInfo(dataInfo)
{
   std::vector< boost::shared_ptr<Property> > paramsCopy(params);
   sample( paramsCopy, m_cases);
}

void Experiment::sample( std::vector< boost::shared_ptr<Property> > & parameterDefinitions, std::vector< Case > & allCases )
{
   if(parameterDefinitions.empty())
      return;

   boost::shared_ptr<Property> lastParameterDefinition = parameterDefinitions.back();
   parameterDefinitions.pop_back(); 

   sample(parameterDefinitions, allCases);

   std::vector< Case > newOutput;

   for (unsigned i = 0; i < allCases.size(); ++i)
   {
      lastParameterDefinition->reset();
      while (! lastParameterDefinition->isPastEnd())
      {
         Case newCase = allCases[i];
         lastParameterDefinition->createParameter(newCase);
         newOutput.push_back( newCase );
         lastParameterDefinition->nextValue();
      }
   }

   allCases.swap( newOutput );
}

std::string Experiment::workingProjectFileName(unsigned caseNumber) const
{
   std::ostringstream fileName;
   fileName 
         << m_experimentInfo.getOutputDirectory()
         << '/'
         << m_experimentInfo.getOutputFileNamePrefix()
         << "_" << caseNumber + 1 
         << ".project3d";
   return fileName.str();
}

std::string Experiment::resultsFileName(unsigned caseNumber) const
{
   std::ostringstream fileName;
   fileName 
         << m_experimentInfo.getOutputDirectory()
         << '/'
         << "Datadriller_"
         << m_experimentInfo.getOutputFileNamePrefix()
         << "_" << caseNumber + 1 
         << ".project3d.dat";
   return fileName.str();
}

struct CreateProjectSetException : formattingexception :: BaseException< CreateProjectSetException > {};

void Experiment :: createProjectsSet() const
{
   // check whether directory exist and is empty
   const std::string workingDir =  m_experimentInfo.getOutputDirectory();
   if ( !directoryExists(workingDir) || !directoryIsEmpty(workingDir) )
      throw CreateProjectSetException() << "Path '" << workingDir << "' must be an existing, empty directory";

   // memorize the directory containing the template project
   const std::string templateDir = getParentDirectory( m_experimentInfo.getTemplateProjectFile() );

   // copy input maps that template project depends on
   std::auto_ptr<DataAccess::Interface::ProjectHandle> templateProject(
      DataAccess::Interface::OpenCauldronProject( m_experimentInfo.getTemplateProjectFile(), "r")
   );
   if (!templateProject.get())
      throw CreateProjectSetException() << "Cannot load template project file '" 
         << m_experimentInfo.getTemplateProjectFile() << "'";

   ProjectDependencies deps = getProjectDependencies( templateProject->getDataBase() );
   for (unsigned i = 0; i < deps.inputMaps.size(); ++i)
      copyFile( templateDir + '/' + deps.inputMaps[i], workingDir + '/' + deps.inputMaps[i]);

   // generate project3d files
   for (unsigned i = 0; i < m_cases.size(); ++i)
      m_cases[i].createProjectFile( m_experimentInfo.getTemplateProjectFile(), workingProjectFileName(i));
}


void Experiment :: runProjectSet() 
{
   const std::string version = "2012.1008";
   const std::string fastcauldronPath = "fastcauldron";
   const std::string runtimeParams = "-temperature";

   // Start an OpenMP thread pool
   #pragma omp parallel
   {
      // Execute the iterations of following for-loop in parallel
      // The 'schedule(...)' bit says that each thread pulls 1 
      // iteration at a time from the work-queue.
      #pragma omp for schedule(dynamic, 1)
      for (unsigned i = 0; i < m_cases.size(); ++i)
      {
         std::ostringstream command;
         command << fastcauldronPath 
                 << " -v" << version
                 << " -project " << workingProjectFileName(i)
                 << ' ' << runtimeParams;
       
         system( command.str().c_str() );
      }
   }
}



void Experiment :: collectResults() const
{

   for (unsigned i=0; i < m_cases.size(); ++i)
   {
      std::ofstream ofs( resultsFileName(i).c_str(), std::ios_base::out | std::ios_base::trunc );
      ofs << "Datamining from project " << workingProjectFileName(i) << " :\n";

      std::vector<double> zs;

      m_probes[0].readDepth(zs);
      ofs << "Depths " << " ";
      for (size_t l = 0; l < zs.size(); ++l)
         ofs << zs[l] << " ";

      ofs << '\n';

      for (unsigned j = 0; j < m_probes.size(); ++j)
      {
         std::vector<double> results;
         m_probes[j].readResults(workingProjectFileName(i), results );

         ofs << m_probes[j].getName() << " ";
         for (size_t k = 0; k < results.size(); ++k)
            ofs << results[k] << " ";

         ofs << '\n';
      }
   }
}



void Experiment::printCases( std::ostream & output ) const
{
   if (m_cases.empty())
   {
      output << "Experiment has no cases\n";
   }
   else
   {
      output << "Cases of experiment\n";
      for (int i=0; i < m_cases.size(); ++i)
      {
         output << std::setw(3) << i+1 << ") ";
         m_cases[i].printParameters(output);
         output << '\n';
      }
   }
}
