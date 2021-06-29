//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CmdRunDataDigger.h"

#include "casaAPI.h"
#include "CasaCommander.h"
#include "CauldronEnvConfig.h"
#include "LogHandler.h"
#include "RunCase.h"

#include <cstdlib>
#include <iostream>

static void PrintObsValues( const casa::RunCaseSet & runCaseSet )
{
   for ( size_t runCaseIndex = 0; runCaseIndex < runCaseSet.size(); ++runCaseIndex )
   {
      // Go through all run cases
      const casa::RunCase* runCase = runCaseSet.runCase(runCaseIndex);

      if ( !runCase ) continue;

      LogHandler( LogHandler::DEBUG_SEVERITY ) << "    " << runCase->projectPath();
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "      Observable values:";

      for ( size_t i = 0; i < runCase->observablesNumber(); ++i )
      {
         const casa::ObsValue * observableValue = runCase->obsValue( i );
         if ( observableValue && observableValue->parent() && observableValue->isDouble() )
         {
            const std::vector<double>      & values = observableValue->asDoubleArray();
            const std::vector<std::string> & names = observableValue->parent()->name();

            for ( size_t j = 0; j < values.size(); ++j )
            {
               LogHandler( LogHandler::DEBUG_SEVERITY ) << "      " << names[j] << " = " << values[j];
            }
         }
      }
   }
}

CmdRunDataDigger::CmdRunDataDigger( CasaCommander & parent, const std::vector< std::string > & commandParameters ) :
  CasaCmd( parent, commandParameters )
{
}

void CmdRunDataDigger::execute( std::unique_ptr<casa::ScenarioAnalysis> & scenarioAnalysis )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Adding jobs to the queue and generating scripts...";

   scenarioAnalysis->runManager().setClusterName("LOCAL");
   scenarioAnalysis->resetRunManager();
   casa::RunManager& runManager = scenarioAnalysis->runManager();
   casa::RunCaseSet& runCaseSet = scenarioAnalysis->doeCaseSet();   

   for ( size_t i = 0; i < runCaseSet.size(); ++i )
   {
     casa::RunCase* runCase = runCaseSet.runCase(i);
     runCase->loadProject();
   }

   if ( ErrorHandler::NoError != runManager.addApplication( new casa::CauldronApp( "datadriller", false ) ) )
   {
      throw ErrorHandler::Exception( runManager.errorCode() ) << runManager.errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Data digger requesting observables...";
   if ( ErrorHandler::NoError != scenarioAnalysis->requestDataDiggerObservables(runCaseSet ) )
   {
      throw ErrorHandler::Exception( scenarioAnalysis->dataDigger().errorCode() ) << scenarioAnalysis->dataDigger().errorMessage();
   }

   // Submit jobs
   for ( size_t i = 0; i < runCaseSet.size(); ++i )
   {
      bool isExcluded;
      scenarioAnalysis->runCaseIsExcluded(i, isExcluded);
      casa::RunCase* runCase = runCaseSet.runCase(i);
      if (runCase->runStatus() == casa::RunCase::Failed || isExcluded)
      {
        LogHandler( LogHandler::WARNING_SEVERITY ) << "  Run case " << i << " failed, hence no data digging case is sceduled";
        continue;
      }
      runCase->setRunStatus(casa::RunCase::NotSubmitted);

      LogHandler( LogHandler::DEBUG_SEVERITY ) << "  Generating run scripts in: " << runCase->projectPath();

      if ( ErrorHandler::NoError != runManager.scheduleCase( *runCase, scenarioAnalysis->scenarioID() ) )
      {
         throw ErrorHandler::Exception( runManager.errorCode() ) << runManager.errorMessage();
      }
   }

   // Spawn jobs for calculation
   if ( ErrorHandler::NoError != runManager.runScheduledCases(1) )
   {
      throw ErrorHandler::Exception( runManager.errorCode() ) << runManager.errorMessage();
   }

   // Collect observable values
   if ( ErrorHandler::NoError != scenarioAnalysis->collectDataDiggerRunResults(runCaseSet) )
   {
      throw ErrorHandler::Exception( runManager.errorCode() ) << runManager.errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Scenario execution succeeded";

   PrintObsValues( runCaseSet );
}

void CmdRunDataDigger::printHelpPage( const char * commandName )
{
  std::cout << "  " << commandName << "\n\n";
  std::cout << "  - Runs the datadigger after adding observables.\n";
  std::cout << "    This command should be executed after the run of DoE generated cases, \n";
  std::cout << "    or a loadstate with finished cases\n";
  std::cout << "    Examples:\n";
  std::cout << "    " << commandName << "\n";
}


