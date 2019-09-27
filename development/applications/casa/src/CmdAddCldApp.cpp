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
#include "CmdAddCldApp.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include <cstdlib>
#include <iostream>

CmdAddCldApp::CmdAddCldApp( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Wrong parameters number: "
         << m_prms.size() << " in Cauldron application definition";
   }

   // set it to negative that later we can check - was it given or not
   m_cpus = -1;
   m_maxRunLimMin = 0;

   size_t it = 0;

   // read cpus number if was given
   if ( m_prms.size() > 1 && CfgFileParser::isNumericPrm( m_prms[it] ) )
   {
      m_cpus = atol( m_prms[it].c_str() );
      if ( m_cpus < 1 || m_cpus > 100000 ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) <<
         "Wrong number of cpus: " << m_cpus << ", for application: " << m_prms[it+1];

      ++it;
   }

   if ( m_prms.size() > 2 &&  CfgFileParser::isNumericPrm( m_prms[it] ) )
   {
      m_maxRunLimMin = atol( m_prms[it].c_str() );
      ++it;
   }

   if (      m_prms[it] == "fastcauldron" ) m_app = casa::RunManager::fastcauldron;
   else if ( m_prms[it] == "fastgenex6"   ) m_app = casa::RunManager::fastgenex6;
   else if ( m_prms[it] == "fastctc"      ) m_app = casa::RunManager::fastctc;
   else if ( m_prms[it] == "fasttouch7"   ) m_app = casa::RunManager::fasttouch7;
   else if ( m_prms[it] == "fastmig"      ) m_app = casa::RunManager::fastmig;
   else if ( m_prms[it] == "tracktraps"   ) m_app = casa::RunManager::tracktraps;
   else if ( m_prms[it] == "track1d"      ) m_app = casa::RunManager::track1d;
   else if ( m_prms[it] == "generic"      ) m_app = casa::RunManager::generic;
   else if ( m_prms[it] == "casa"         ) m_app = casa::RunManager::casa;
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown cauldron application name: " << m_prms[0];
   }
}

void CmdAddCldApp::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   // was cpus number and time limit given?
   size_t p = m_cpus < 0 ? 1 : (m_maxRunLimMin == 0 ? 2 : 3);
   if ( m_cpus < 0 ) { m_cpus = 1; } // if not - set it to 1

   LogHandler( LogHandler::INFO_SEVERITY ) << "Add cauldron application to calculation pipeline " << m_prms[p - 1] << "(" <<
                                 CfgFileParser::implode( m_prms, ",", p ) << ")";

   casa::CauldronApp * app = 0;

   if ( casa::RunManager::generic == m_app )
   {
      const std::string & appName = m_prms[p++];

      app = casa::RunManager::createApplication( casa::RunManager::generic, m_cpus, m_maxRunLimMin, appName );
   }
   else
   {
      app = casa::RunManager::createApplication( static_cast<casa::RunManager::ApplicationType>( m_app ), m_cpus, m_maxRunLimMin );
   }

   assert( 0 != app );

   for ( ; p < m_prms.size(); ++p ) app->addOption( m_prms[p] );

   casa::RunManager & rm = sa->runManager();
   if ( ErrorHandler::NoError != rm.addApplication( app ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }
}


void CmdAddCldApp::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName <<
   R"( [<cpus>] [<timeLim>] "<appName>" ["appPrm1" ["appPrm2"]... ]
      - add a new Cauldron application to the end of the simulation pipeline. The full Cauldron simulation
        could consists of several stages, like P/T simulation then genex and then migration.
           cpus    - (Optional) If application is parallel, user could specify number of cpus. By default,
                     the number of cpus is set to 1. If a user is intended to define a time limits (see next
                     parameter description) he must define the cpus number to some value.
           timeLim - (Optional) Defines the time limits for a specific application. If the wall clock
                     exceeds timeLim, the application will be stopped
                     and all further stages in applications pipeline will be invalidated also.
           appName - name of the cauldron application. Here is a list of supported applications:
                     "fastcauldron"  - P/T calculation
                     "fastgenex6"    - source rock HC generation calculation
                     "fastctc"       - crustal thickness calculator
                     "fasttouch7"    - reservoir quality calculator
                     "fastmig"       - HC migration simulator
                     "tracktraps"    - traps tracking over time
                     "track1d"       - extracting properties value along vertical well
          appPrmN  - (Optional) application parameters. Input and output project file names must not be specified
                     as command options, they will be added by CASA.

        Here is an examples of using ")" << cmdName << "\" command:\n";
   std::cout << "         " << cmdName << " fastcauldron \"-itcoupled\"\n";
   std::cout << "         " << cmdName << " 4 fastcauldron \"-itcoupled\"\n";
   std::cout << "         " << cmdName << " 4 20 fastcauldron \"-itcoupled\"   #4 cpus run max 20 minutes\n";
   std::cout << "         " << cmdName << " fastgenex6\n";
}


