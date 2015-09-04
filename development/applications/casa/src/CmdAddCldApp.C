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
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown cauldron application name: " << m_prms[0];
   }
}

void CmdAddCldApp::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
#ifdef _WIN32
   casa::CauldronApp::ShellType sh = casa::CauldronApp::cmd;
#else
   casa::CauldronApp::ShellType sh = casa::CauldronApp::bash;
#endif // _WIN32

   // was cpus number given?
   size_t p = m_cpus < 0 ? 1 : 2;
   if ( m_cpus < 0 ) { m_cpus = 1; } // if not - set it to 1

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Add cauldron application to calculation pipeline " << m_prms[p - 1] << "(";

      for ( size_t i = p; i < m_prms.size(); ++i ) std::cout << m_prms[i] << ((i == m_prms.size() - 1) ? "" : ",");
      
      std::cout << ")" << std::endl;
   }
   casa::CauldronApp * app = 0;
   

   if ( casa::RunManager::generic == m_app )
   {
      const std::string & appName = m_prms[p++];

      app = casa::RunManager::createApplication( casa::RunManager::generic, m_cpus, m_maxRunLimMin, sh, appName );
   }
   else { 
   app = casa::RunManager::createApplication( static_cast<casa::RunManager::ApplicationType>( m_app ), m_cpus, m_maxRunLimMin, sh ); }

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
   std::cout << "  " << cmdName << " [cpus] <app name> [app parameters]\n";
   std::cout << "   - add a new Cauldron application at the end of the simulation pipeline. The full Cauldron simulation \n";
   std::cout << "     could consists of several stages, like P/T simulation then genex and then migration. Command must \n";
   std::cout << "     have an application name as the first parameter and optionally, application parameters. Input and \n";
   std::cout << "     output project file names must not be specified as command options, they will be added by CASA.\n";
   std::cout << "     If application is parallel, user could specify number of cpus before the application name. This is an \n";
   std::cout << "     optional parameter, if it was not specified the number of cpus will be set to 1.\n";
   std::cout << "     Here is an examples of using \"" << cmdName << "\" command:\n";
   std::cout << "         " << cmdName << " fastcauldron \"-itcoupled\"\n";
   std::cout << "         " << cmdName << " 4 fastcauldron \"-itcoupled\"\n";
   std::cout << "         " << cmdName << " fastgenex6\n";
}


