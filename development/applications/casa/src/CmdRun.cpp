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

#include <cstdlib>
#include <iostream>

static void PrintObsValues( casa::ScenarioAnalysis & sc )
{
   casa::RunCaseSet & rcSet = sc.doeCaseSet();

   for ( size_t rc = 0; rc < rcSet.size(); ++rc )
   {
      // go through all run cases
      const casa::RunCase * cs = rcSet[rc];

      if ( !cs ) continue;

      LogHandler( LogHandler::DEBUG ) << "    " << cs->projectPath();
      LogHandler( LogHandler::DEBUG ) << "      Observable values:";

      for ( size_t i = 0; i < cs->observablesNumber(); ++i )
      {
         casa::ObsValue * ov = cs->obsValue( i );
         if ( ov && ov->observable() && ov->isDouble() )
         {
            const std::vector<double>      & vals  = ov->asDoubleArray();
            const std::vector<std::string> & names = ov->observable()->name();

            for ( size_t i = 0; i < vals.size(); ++i )
            {
               LogHandler( LogHandler::DEBUG ) << "      " << names[i] << " = " << vals[i];
            }
         }
      }
   }
}

CmdRun::CmdRun( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_cluster        = m_prms.size() > 0 ? m_prms[0] : "";
   m_cldVer         = m_prms.size() > 1 ? m_prms[1] : "";
   m_maxPendingJobs = 0;

   if ( m_prms.size() > 2 )
   {
      if ( CfgFileParser::isNumericPrm( m_prms[2] ) ) { m_maxPendingJobs = atol( m_prms[2].c_str() ); }
      else                                            { m_resStr = m_prms[2];                         }
   }
   if ( m_prms.size() > 3 ) { m_resStr = m_prms[3]; }

   if ( m_cluster.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty HPC cluster name";

   if ( m_cldVer.empty() || m_cldVer == "Default" ) 
   {
      m_cldVer = ibs::Path::applicationFullPath().path();
   }
}

void CmdRun::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO ) << "Adding jobs to the queue and generating scripts...";

   casa::RunManager & rm = sa->runManager();

   // set version and cluster
   if ( ErrorHandler::NoError != rm.setCauldronVersion( m_cldVer.c_str() ) ||
        ErrorHandler::NoError != rm.setClusterName( m_cluster.c_str() )
      ) throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();

   if ( m_maxPendingJobs > 0 ) rm.setMaxNumberOfPendingJobs( m_maxPendingJobs );
   if ( !m_resStr.empty()    ) rm.setResourceRequirements( m_resStr );

   // submit jobs
   for ( size_t i = 0; i < sa->doeCaseSet().size(); ++i )
   {
      LogHandler( LogHandler::DEBUG ) << "  Generating run scripts in: " << (sa->doeCaseSet()[i])->projectPath();

      if ( ErrorHandler::NoError != rm.scheduleCase( *(sa->doeCaseSet()[i]), sa->scenarioID() ) )
      {
         throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
      }
   }

   LogHandler( LogHandler::INFO ) << "Submitting jobs to the cluster " << m_prms[0] << " using Cauldron: " << m_prms[1];

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
   
   LogHandler( LogHandler::INFO ) << "Scenarion execution succeeded";

   PrintObsValues( *sa.get() );
}

void CmdRun::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <HPC cluster name> <Cauldron version> [<max number of pending jobs>] [<hostSpec>]\n\n";
   std::cout << "  - generates script files for each case and submit jobs to HPC cluster for execution.\n";
   std::cout << "    If cluster name is defined as LOCAL, jobs will be submitted to the local host. The number\n";
   std::cout << "    of jobs submitted at one time will be restricted by the number of cores on local host.\n";
   std::cout << "    Max number of pending jobs optional parameter allows to keep the number of pending jobs on HPC\n";
   std::cout << "    below or equal the given value. New jobs will not be submitted till pending jobs will start\n";
   std::cout << "    This allows to follow a \"fair-share\" policy on HPC cluster.\n";
   std::cout << "    Here:\n";
   std::cout << "    <HPC cluster name> - cluster name, LOCAL | AMSGDC | HOUGDC. \n";
   std::cout << "    <Cauldron version> - simulator version. Must be installed in IBS folder. Could be specified as \"Default\".\n";
   std::cout << "                         In this case the same simulator version as casa application will be used.\n";
   std::cout << "    <max number of pending jobs> - (Optional) if this parameter is specified, casa will not submit jobs till the number of\n";
   std::cout << "                                   pending jobs on cluster is bigger the the given value\n";
   std::cout << "    <hostSpec>         - LSF resorces selection string. It is in the same format as bsub parameter -R\n";

   std::cout << "    User could customize LSF options throug envinronment variables: \n";

   const char * envVar = getenv( "LSF_CAULDRON_PROJECT_NAME" ); 
   std::cout << "    LSF_CAULDRON_PROJECT_NAME - project name (bsub -P ...)\n";
   std::cout << "        the default value: " << LSF_CAULDRON_PROJECT_NAME << "\n";
   std::cout << "        the current value: " << ( envVar ? envVar : LSF_CAULDRON_PROJECT_NAME ) << "\n";
      
   envVar = getenv( "LSF_CAULDRON_PROJECT_QUEUE" ); 
   std::cout << "    LSF_CAULDRON_PROJECT_QUEUE - project queue (bsub -q .. )\n";
   std::cout << "        the default value: NotDefined" << "\n";
   std::cout << "        the current value: " << ( envVar ? envVar : "NotDefined" ) << "\n";

   envVar = getenv( "LSF_CAULDRON_PROJECT_GROUP" ); 
   std::cout << "    LSF_CAULDRON_PROJECT_GROUP - project group (bsub -G ...)\n";
   std::cout << "        the default value: NotDefined" << "\n";
   std::cout << "        the current value: " << ( envVar ? envVar : "NotDefined" ) << "\n";

   envVar = getenv( "LSF_CAULDRON_PROJECT_SERVICE_CLASS_NAME" );
   std::cout << "    LSF_CAULDRON_PROJECT_SERVICE_CLASS_NAME - project class service name (bsla)\n";
   std::cout << "        the default value: NotDefined" << "\n";
   std::cout << "        the current value: " << ( envVar ? envVar : "NotDefined" ) << "\n";

   std::cout << "\n";
   std::cout << "    Examples:\n";
   std::cout << "    #     Cluster  Cauldron version.\n";
   std::cout << "    " << cmdName << " \"LOCAL\"        \"v2015.1104\"\n";
   std::cout << "\n";
   std::cout << "    #     Cluster  Cauldron ver.   Max pend. jobs\n";
   std::cout << "    " << cmdName << " \"AMSGDC\"        \"v2015.1104\"  5\n";
   std::cout << "\n";
   std::cout << "    #     Cluster  Cauldron ver.   Max pend. jobs\n";
   std::cout << "    " << cmdName << " \"AMSGDC\"        \"v2016.05nightly\"  5  \"select[ostype==RHEL6.6]\"\n";
   std::cout << "\n";
}


