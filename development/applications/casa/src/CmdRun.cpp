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

#include "LogHandler.h"

#include "CauldronEnvConfig.h"

#include "utilities.h"

#include <cstdlib>
#include <iostream>

CmdRun::CmdRun( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_cluster        = m_prms.size() > 0 ? m_prms[0] : "";
   m_cldVer         = m_prms.size() > 1 ? m_prms[1] : "";
   m_maxPendingJobs = 0;

   if ( m_prms.size() > 2 )
   {
      if ( CfgFileParser::isNumericPrm( m_prms[2] ) ) { m_maxPendingJobs = atol( m_prms[2].c_str() ); }
   }

   if ( m_cluster.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty HPC cluster name";

   casaAppUtils::checkCauldronVersion(m_cldVer);
}

void CmdRun::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Adding jobs to the queue and generating scripts...";

   casa::RunManager & rm = sa->runManager();

   // set version and cluster
   if ( ErrorHandler::NoError != rm.setCauldronVersion( m_cldVer.c_str() ) ||
        ErrorHandler::NoError != rm.setClusterName( m_cluster.c_str() )
      ) throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();

   if ( m_maxPendingJobs > 0 ) rm.setMaxNumberOfPendingJobs( m_maxPendingJobs );

   // submit jobs
   for ( size_t i = 0; i < sa->doeCaseSet().size(); ++i )
   {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "  Generating run scripts in: " << (sa->doeCaseSet()[i])->projectPath();

      if ( ErrorHandler::NoError != rm.scheduleCase( *(sa->doeCaseSet()[i]), sa->scenarioID() ) )
      {
         throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
      }
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Submitting jobs to the cluster " << m_cluster << " using Cauldron: " << m_cldVer;

   // spawn jobs for calculation
   if ( ErrorHandler::NoError != rm.runScheduledCases() )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Scenario execution succeeded";
}

void CmdRun::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << R"( <HPC cluster name> <Cauldron version> [<max number of pending jobs>] [<hostSpec>]

     - generates script files for each case and submit jobs to HPC cluster for execution.
       If cluster name is defined as LOCAL, jobs will be submitted to the local host. The number
       of jobs submitted at one time will be restricted by the number of cores on local host.
       Max number of pending jobs optional parameter allows to keep the number of pending jobs on HPC
       below or equal the given value. New jobs will not be submitted till pending jobs will start
       This allows to follow a "fair-share" policy on HPC cluster.

       Here:
       <HPC cluster name> - cluster name, LOCAL | AMSGDC | HOUGDC.
       <Cauldron version> - simulator version. Must be installed in IBS folder. Could be specified as "Default"
                            In this case the same simulator version as casa application will be used.
       <max number of pending jobs> - (Optional) if this parameter is specified, casa will not submit jobs till the number of
                                      pending jobs on cluster is bigger the the given value\n)";

   std::cout << "\n";
   std::cout << "    Examples:\n";
   std::cout << "    #     Cluster  Cauldron version.\n";
   std::cout << "    " << cmdName << " \"LOCAL\"        \"v2016.1101\"\n";
   std::cout << "\n";
   std::cout << "    #     Cluster  Cauldron ver.   Max pend. jobs\n";
   std::cout << "    " << cmdName << " \"AMSGDC\"        \"v2016.1101\"  5\n";
   std::cout << "\n";
}


