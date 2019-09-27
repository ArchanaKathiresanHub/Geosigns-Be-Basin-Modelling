//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "CasaCommander.h"
#include "CmdRunBaseCase.h"

#include "casaAPI.h"
#include "RunCase.h"

#include "LogHandler.h"

#include <cstdlib>
#include <iostream>

static void PrintObsValues( const casa::RunCase * cs )
{
   if ( !cs ) return;

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "    " << cs->projectPath();
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "      Observable values:";

   for ( size_t i = 0; i < cs->observablesNumber(); ++i )
   {
      const casa::ObsValue * ov = cs->obsValue( i );
      if ( ov && ov->parent() && ov->isDouble() )
      {
         const std::vector<double>      & vals  = ov->asDoubleArray();
         const std::vector<std::string> & names = ov->parent()->name();
         for ( size_t i = 0; i < vals.size(); ++i )
         {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "      " << names[i] << " = " << vals[i];
         }
      }
   }
}


CmdRunBaseCase::CmdRunBaseCase( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
}

void CmdRunBaseCase::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Data digger requesting observables for base case...";

   if ( ErrorHandler::NoError != sa->dataDigger().requestObservables( sa->obsSpace(), sa->baseCaseRunCase() ) )
   {
      throw ErrorHandler::Exception( sa->dataDigger().errorCode() ) << sa->dataDigger().errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Adding base case project to the queue and generating scripts...";

   casa::RunManager & rm = sa->runManager();
   
   // submit the job
   if ( ErrorHandler::NoError != rm.scheduleCase( *(sa->baseCaseRunCase()), sa->scenarioID()) )
   {
         throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Submitting jobs for base case to the cluster: " << rm.clusterName() << "...";

   // spawn jobs for calculation
   if ( ErrorHandler::NoError != rm.runScheduledCases() )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   if ( sa->baseCaseRunCase()->runStatus() == casa::RunCase::Completed )
   {
      LogHandler( LogHandler::INFO_SEVERITY ) << "Collecting observables values...";
      // collect observables value
      if ( ErrorHandler::NoError != sa->dataDigger().collectRunResults( sa->obsSpace(), sa->baseCaseRunCase() ) )
      {
         throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
      }
      LogHandler( LogHandler::INFO_SEVERITY ) << "Base case execution succeeded";
   
      PrintObsValues( sa->baseCaseRunCase() );
   }
   else
   {
      LogHandler( LogHandler::INFO_SEVERITY ) << "Base case execution failed";
   }
}

void CmdRunBaseCase::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << "\n\n";
   std::cout << "  - generates script files for base case and submit jobs to HPC cluster for execution.\n";
   std::cout << "    This command should be executed after the run of DoE generated cases. \n";
   std::cout << "    It uses the same cluster name and Cauldron applications version\n";
   std::cout << "    Examples:\n";
   std::cout << "    " << cmdName << "\n";
}

