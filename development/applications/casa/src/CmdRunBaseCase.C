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

#include <cstdlib>
#include <iostream>

static void PrintObsValues( const casa::RunCase * cs )
{
   if ( !cs ) return;

   std::cout << "    " << cs->projectPath() << std::endl;
   std::cout << "    Observable values:" << std::endl;

   for ( size_t i = 0; i < cs->observablesNumber(); ++i )
   {
      casa::ObsValue * ov = cs->obsValue( i );
      if ( ov && ov->observable() && ov->isDouble() )
      {
         const std::vector<double>      & vals  = ov->asDoubleArray();
         const std::vector<std::string> & names = ov->observable()->name();
         for ( size_t i = 0; i < vals.size(); ++i )
         {
            std::cout << "      " << names[i] << " = " << vals[i] << "\n";
         }
      }
   }
}



CmdRunBaseCase::CmdRunBaseCase( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
}

void CmdRunBaseCase::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Data digger requesting observables for base case ..." << std::endl;
   }

   if ( ErrorHandler::NoError != sa->dataDigger().requestObservables( sa->obsSpace(), sa->baseCaseRunCase() ) )
   {
      throw ErrorHandler::Exception( sa->dataDigger().errorCode() ) << sa->dataDigger().errorMessage();
   }

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Adding base case project to the queue and generating scripts..." << std::endl;
   }

   casa::RunManager & rm = sa->runManager();
   
   // submit the job
   if ( ErrorHandler::NoError != rm.scheduleCase( *(sa->baseCaseRunCase()) ) )
   {
         throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Submitting jobs for base case to the cluster: " << rm.clusterName() << std::endl;
   }

   // spawn jobs for calculation
   if ( ErrorHandler::NoError != rm.runScheduledCases( false ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   // collect observables value
   if ( ErrorHandler::NoError != sa->dataDigger().collectRunResults( sa->obsSpace(), sa->baseCaseRunCase() ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   if ( m_commander.verboseLevel() > CasaCommander::Minimal ) { PrintObsValues( sa->baseCaseRunCase() ); }
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

