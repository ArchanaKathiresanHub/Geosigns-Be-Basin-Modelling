#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <memory>

#include "experiment.h"

#include "BasementProperty.h"
#include "RuntimeConfiguration.h"
#include "DatadrillerProperty.h"
#include "Scenario.h"
#include "projectdependencies.h"
#include "formattingexception.h"
#include "system.h"
#include "ExperimentResultsTable.h"

#include "Interface/ProjectHandle.h"

Experiment :: Experiment( const std::vector< boost::shared_ptr<Property> > & params, const std::vector<DatadrillerProperty> & datadrillerDefinitions, const RuntimeConfiguration & dataInfo)
   : m_scenarios( sample(params) )
   , m_probes(datadrillerDefinitions)
   , m_experimentInfo(dataInfo)
{
}

std::vector< Scenario > Experiment::sample( const std::vector< boost::shared_ptr<Property> > & properties)
{
   std::vector< Scenario > scenarios(1);
   std::vector< Scenario > newScenarios;
   for (unsigned p = 0 ; p < properties.size(); ++p)
   {
      Property & property = *properties[p];

      newScenarios.clear();
      for (unsigned i = 0; i < scenarios.size(); ++i)
      {
         for( property.reset(); ! property.isPastEnd(); property.nextValue())
         {
            newScenarios.push_back( scenarios[i] );
            property.createParameter( newScenarios.back() );
         }
      }

      newScenarios.swap( scenarios );
   }

   return scenarios;
}

std::string Experiment::workingProjectFileName(unsigned scenarioNumber) const
{
   std::ostringstream fileName;
   fileName 
         << m_experimentInfo.getOutputDirectory()
         << '/'
         << m_experimentInfo.getOutputFileNamePrefix()
         << "_" << scenarioNumber + 1 
         << ".project3d";
   return fileName.str();
}

std::string Experiment::workingLogFileName(unsigned scenarioNumber) const
{
   std::ostringstream fileName;
   fileName 
         << m_experimentInfo.getOutputDirectory()
         << "/Log_" << scenarioNumber + 1 
         << ".txt";
   return fileName.str();
}

std::string Experiment::resultsFileName(unsigned scenarioNumber) const
{
   std::ostringstream fileName;
   fileName 
         << m_experimentInfo.getOutputDirectory()
         << '/'
         << "Datadriller_"
         << m_experimentInfo.getOutputFileNamePrefix()
         << "_" << scenarioNumber + 1 
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
   for (unsigned i = 0; i < m_scenarios.size(); ++i)
      try
      {
         m_scenarios[i].createProjectFile( m_experimentInfo.getTemplateProjectFile(), workingProjectFileName(i));
      }
      catch (formattingexception::GeneralException & fe)
      {
	 std::cerr << std::endl;
	 std::cerr << "Error in scenario " << i << ": ";
	 std::cerr << fe.what () << std::endl;
	 std::cerr << "Will ignore this scenario" << std::endl;
	 std::cerr << std::endl;
      }
}


void Experiment :: runProjectSet( std::ostream * verboseOutput) 
{
   const std::string fastcauldronPath = "fastcauldron";
   const std::string runtimeParams = m_experimentInfo.getCauldronRuntimeParams();
   const std::string cauldronVersion = m_experimentInfo.getCauldronVersion();

   int scenariosFinished = 0;

   // Start an OpenMP thread pool
   #pragma omp parallel
   {
      // Execute the iterations of following for-loop in parallel
      // The 'schedule(...)' bit says that each thread pulls 1 
      // iteration at a time from the work-queue.
      #pragma omp for schedule(dynamic, 1)
      for (unsigned i = 0; i < m_scenarios.size(); ++i)
      {
	 if (verboseOutput)
	 {
	    #pragma omp critical(printing)
            if (m_scenarios[i].isValid ())
	    {
	       *verboseOutput << "Starting scenario " << i + 1 << endl;
	    }
            else
            {
	       *verboseOutput << "Skipping scenario " << i + 1 << endl;
	    }
	 }

         if (!m_scenarios[i].isValid ()) continue;

         std::ostringstream command;
         command << fastcauldronPath 
                 << " -v" << cauldronVersion
                 << " -project " << workingProjectFileName(i)
                 << ' ' << runtimeParams
		 << " > " <<  workingLogFileName (i) << " 2>&1";
       
         system( command.str().c_str() );

	 if (verboseOutput)
	 {
	    #pragma omp critical(printing)
	    {
	       *verboseOutput << "Finished scenario " << i + 1 << std::endl;
               ++scenariosFinished;
	    }
	 }
      }
   }
   if (verboseOutput)
   {
      *verboseOutput << "\nFinished " << scenariosFinished << ", skipped " << m_scenarios.size () - scenariosFinished << " scenarios " << std::endl;
   }
}


void Experiment :: collectResults() const
{
   std::vector< std::string > probeNames(m_probes.size());
   for (unsigned i = 0; i < m_probes.size(); ++i)
      probeNames[i] = m_probes[i].getName();

   ExperimentResultsTable table( probeNames,
            m_experimentInfo.getOutputTableFieldSeparator(),
            m_experimentInfo.getOutputTableFixedWidth() 
         );
   std::vector< double > zs, values;

   for (unsigned i=0; i < m_scenarios.size(); ++i)
   {
      if (!m_scenarios[i].isValid ()) continue;

      // gather all data of all probes in one big table
      table.clear();
      for (unsigned j = 0; j < m_probes.size(); ++j)
      {
         double x, y, age; 
         m_probes[j].readResults( workingProjectFileName(i), x, y, zs, age, values);

         assert( zs.size() == values.size() );

         for (unsigned k = 0; k < zs.size(); ++k)
            table.add( ExperimentResultsTable::Entry( 
                     ExperimentResultsTable::PositionAndTime( x, y, zs[k], age),
                     j, 
                     values[k]
                     ) );
      }

      // output the table to file
      std::ofstream ofs( resultsFileName(i).c_str(), std::ios_base::out | std::ios_base::trunc );
      ofs << "# Results from project " << workingProjectFileName(i) << ":\n";
      table.print( ofs );
   }
}



void Experiment::printScenarios( std::ostream & output ) const
{
   if (m_scenarios.empty())
   {
      output << "Experiment has no scenarios\n";
   }
   else
   {
      output << "Scenarios of experiment\n";
      for (int i=0; i < m_scenarios.size(); ++i)
      {
         output << std::setw(3) << i+1 << ") ";
         m_scenarios[i].printParameters(output);
         output << '\n';
      }
   }
}
