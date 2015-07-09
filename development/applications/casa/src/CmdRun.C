//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "CasaCommander.h"
#include "CmdRun.h"

#include "casaAPI.h"
#include "RunCase.h"

#include <cstdlib>
#include <iostream>


static void PrintObsValues( casa::ScenarioAnalysis & sc )
{
   casa::ObsSpace   & obSpace = sc.obsSpace();
   casa::RunCaseSet & rcSet = sc.doeCaseSet();

   for ( size_t rc = 0; rc < rcSet.size(); ++rc )
   {
      // go through all run cases
      const casa::RunCase * cs = rcSet[rc];

      if ( !cs ) continue;

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
}


CmdRun::CmdRun( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_cluster        = m_prms.size() > 0 ? m_prms[0] : "";
   m_cldVer         = m_prms.size() > 1 ? m_prms[1] : "";
   m_maxPendingJobs = m_prms.size() > 2 ? atol( m_prms[2].c_str() ) : 0;

   if ( m_cluster.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty HPC cluster name";

   if ( m_cldVer.empty() || m_cldVer == "Default" ) 
   {
      m_cldVer = ibs::Path::applicationFullPath().path();
   }
}

void CmdRun::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Adding jobs to the queue and generating scripts for:" << std::endl;
   }

   casa::RunManager & rm = sa->runManager();

   if ( m_maxPendingJobs > 0 ) rm.setMaxNumberOfPendingJobs( m_maxPendingJobs );

   // set version and cluster
   if ( ErrorHandler::NoError != rm.setCauldronVersion( m_cldVer.c_str() ) ||
        ErrorHandler::NoError != rm.setClusterName( m_cluster.c_str() )
      ) throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();

   // submit jobs
   for ( size_t i = 0; i < sa->doeCaseSet().size(); ++i )
   {
      if ( m_commander.verboseLevel() > CasaCommander::Quiet )
      {
         std::cout << "    " << (sa->doeCaseSet()[i])->projectPath() << std::endl;
      }

      if ( ErrorHandler::NoError != rm.scheduleCase( *(sa->doeCaseSet()[i]) ) )
      {
         throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
      }
   }

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Submitting jobs to the cluster " << m_prms[0] << " using Cauldron: " << m_prms[1] << std::endl;
   }

   // spawn jobs for calculation
   if ( ErrorHandler::NoError != rm.runScheduledCases( false ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   // collect observables value
   if ( ErrorHandler::NoError != sa->dataDigger().collectRunResults( sa->obsSpace(), sa->doeCaseSet() ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   if ( m_commander.verboseLevel() > CasaCommander::Minimal ) { PrintObsValues( *sa.get() ); }
}

void CmdRun::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <HPC cluster name> <Cauldron version> [<max number of pending jobs>]\n\n";
   std::cout << "  - generates script files for each case and submit jobs to HPC cluster for execution.\n";
   std::cout << "    If cluster name is defined as LOCAL, jobs will be submitted to the local host. The number\n";
   std::cout << "    of jobs submitted at one time will be restricted by the number of cores on local host.\n";
   std::cout << "    Max number of pending jobs optional parameter allows to keep the number of pending jobs on HPC\n";
   std::cout << "    below or equal the given value. New jobs will not be submitted till pending jobs will start\n";
   std::cout << "    This allows to follow a \"fair-share\" policy on HPC cluster.\n\n";
   std::cout << "\n";
   std::cout << "    Examples:\n";
   std::cout << "    #     Cluster  Cauldron version.\n";
   std::cout << "    " << cmdName << " \"LOCAL\"        \"v2014.07nightly\"\n";
   std::cout << "\n";
   std::cout << "    #     Cluster  Cauldron ver.   Max pend. jobs\n";
   std::cout << "    " << cmdName << " \"AMSGDC\"        \"v2014.07nightly\"  5\n";
   std::cout << "\n";
}

