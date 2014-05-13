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
#include "parameter.h"
#include "Scenario.h"
#include "SAUAParameters.h"
#include "projectdependencies.h"
#include "formattingexception.h"
#include "system.h"
#include "ExperimentResultsTable.h"

#include "Interface/ProjectHandle.h"
#include "formattingexception.h"

#include <Case.h>
#include <BoxBehnken.h>
#include <Tornado.h>
#include <ScreenDesign.h>
#include <OptimisedLHD.h>
#include <ParameterBounds.h>

struct ConfigurationException : formattingexception :: BaseException< ConfigurationException > {};

Experiment::Experiment( const std::vector< boost::shared_ptr<Property> > & params
                      , const std::vector<DatadrillerProperty>           & datadrillerDefinitions
                      , const RuntimeConfiguration                       & dataInfo
                      , const SAUAParameters                             & sauap
                      ) :
     m_probes(datadrillerDefinitions)
   , m_scenarios( sauap.getDoE().empty() ? sample( params ) : createDoE( params, sauap ) )
   , m_experimentInfo(dataInfo)
{
   ;
}


std::vector< Scenario > Experiment::sample( const std::vector< boost::shared_ptr<Property> > & properties )
{
   std::vector< Scenario > scenarios(1);
   std::vector< Scenario > newScenarios;

   for ( size_t p = 0 ; p < properties.size(); ++p )
   {
      Property & property = *properties[p];

      newScenarios.clear();

      for ( size_t i = 0; i < scenarios.size(); ++i )
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

std::vector< Scenario > Experiment::createDoE( const std::vector< boost::shared_ptr<Property> > & properties, const  SAUAParameters & sauap )
{
   Scenario minValScenario;
   Scenario maxValScenario;

   for ( size_t p = 0 ; p < properties.size(); ++p )
   {
      properties[p]->reset();
      properties[p]->createParameter( minValScenario );
      properties[p]->lastValue();
      properties[p]->createParameter( maxValScenario );
   }

   assert( minValScenario.size() == maxValScenario.size() );

   std::vector<double> minBndProp;
   std::vector<double> maxBndProp;

   // convert min values for parameters list
   for ( Scenario::iterator it = minValScenario.begin(); it != minValScenario.end(); ++it )
   {
      if ( it->isContinuous() )
      {
         const std::vector<double> & prms = it->toDblVector();
         if ( prms.size() > 0 )
         {
            minBndProp.insert( minBndProp.end(), prms.begin(), prms.end() );
         }
      }
   }
   // convert max values for parameters list
   for ( Scenario::iterator it = maxValScenario.begin(); it != maxValScenario.end(); ++it )
   {
      if ( it->isContinuous() )
      {
         const std::vector<double> & prms = it->toDblVector();
         if ( prms.size() > 0 )
         {
            maxBndProp.insert( maxBndProp.end(), prms.begin(), prms.end() );
         }
      }
   }

   assert( minBndProp.size() == maxBndProp.size() );

   std::vector<double> baseBndProp;

   for ( size_t i = 0; i < minBndProp.size(); ++i )
   {
      baseBndProp.push_back( (minBndProp[i] + maxBndProp[i]) * 0.5 );
   }

   const std::vector< bool > pSelected( minBndProp.size(), true );

   std::auto_ptr<SUMlib::ExpDesignBase> doe;

   const std::string & doeName = sauap.getDoE();

   if (      doeName == "Box Behnken"     ) { doe.reset( new SUMlib::BoxBehnken(   pSelected, pSelected.size() ) ); }
   else if ( doeName == "Tornado"         ) { doe.reset( new SUMlib::Tornado(      pSelected, pSelected.size() ) ); }
   else if ( doeName == "Plackett-Burman" ) { doe.reset( new SUMlib::ScreenDesign( pSelected, pSelected.size() ) ); }
   else if ( doeName == "Optimised LHD"   ) { doe.reset( new SUMlib::OptimisedLHD( pSelected, pSelected.size(), atoi( sauap.getDoEPrms().c_str() ) ) ); }
   else
   {
      throw ConfigurationException() << "Unknown name of DoE algorithm: " << doeName;
   }

   SUMlib::Case minSumCase;
   SUMlib::Case maxSumCase;
   SUMlib::Case baseSumCase;

   minSumCase.setContinuousPart( minBndProp );
   maxSumCase.setContinuousPart( maxBndProp );
   baseSumCase.setContinuousPart( baseBndProp );

   // create bounds. No categorical values for current implementation
   const SUMlib::ParameterBounds pBounds( minSumCase, maxSumCase, std::vector< SUMlib::IndexList >() );

   std::vector< SUMlib::Case > bbCaseSet;

   doe->getCaseSet( pBounds, baseSumCase, true, bbCaseSet );

   std::vector< Scenario > scenarios( bbCaseSet.size() );

   size_t ci = 0;
   for ( std::vector<SUMlib::Case>::const_iterator c = bbCaseSet.begin(); c != bbCaseSet.end(); ++c )
   {
      for ( size_t p = 0 ; p < properties.size(); ++p )
      {
         properties[p]->reset();
         properties[p]->createParameter( scenarios[ci] ); // creat some default value for scenario
      }

      // go over all paramertes in scenario and set up DoE calculated parameters
      const SUMlib::Case      & sumCas = *c;
      const std::vector<double> case_prms  = (*c).continuousPart();

      size_t prmPos = 0;

      for ( Scenario::iterator it = scenarios[ci].begin(); it != scenarios[ci].end(); ++it )
      {
         if ( it->isContinuous() )
         {
            std::vector<double> scen_prms = it->toDblVector();

            if ( scen_prms.size() > 0 )
            {
               std::copy( case_prms.begin() + prmPos, case_prms.begin() + prmPos + scen_prms.size(), scen_prms.begin() );
            }
            prmPos += scen_prms.size();

            //update scenario parameter
            it->fromDblVector( scen_prms );
         }
      }
      ci++;
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

   if ( !templateProject.get() )
   {
      throw CreateProjectSetException() << "Cannot load template project file '" << m_experimentInfo.getTemplateProjectFile() << "'";
   }

   ProjectDependencies deps = getProjectDependencies( templateProject->getDataBase() );

   for ( unsigned i = 0; i < deps.inputMaps.size(); ++i )
      copyFile( templateDir + '/' + deps.inputMaps[i], workingDir + '/' + deps.inputMaps[i]);

   // generate project3d files
   for ( unsigned i = 0; i < m_scenarios.size(); ++i )
   {
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


void Experiment::collectResults() const
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
