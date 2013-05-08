#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "experiment.h"

#include "BasementProperty.h"
#include "RuntimeConfiguration.h"
#include "DatadrillerProperty.h"
#include "case.h"

#include <unistd.h>

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
   if (parameterDefinitions.empty())
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

void Experiment :: createProjectsSet() const
{
   // TODO: Remove old directory? Copy input maps of project file?

   for (unsigned i = 0; i < m_cases.size(); ++i)
      m_cases[i].createProjectFile( m_experimentInfo.getTemplateProjectFile(), workingProjectFileName(i));
}


void Experiment :: runProjectSet() 
{
   const std::string version = "2012.1008";
   const std::string fastcauldronPath = "fastcauldron";
   const std::string runtimeParams = "-temperature";

//  #pragma omp parallel for
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


void Experiment :: collectResults() const
{
   for (unsigned i=0; i < m_cases.size(); ++i)
   {
      std::ofstream ofs( resultsFileName(i).c_str(), std::ios_base::out | std::ios_base::trunc );
      ofs << "Datamining from project " << workingProjectFileName(i) << " :\n";

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
