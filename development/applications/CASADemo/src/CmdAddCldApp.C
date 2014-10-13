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
   assert( m_prms.size() > 0 );

   if (      m_prms[0] == "fastcauldron" ) m_app = casa::RunManager::fastcauldron;
   else if ( m_prms[0] == "fastgenex6"   ) m_app = casa::RunManager::fastgenex6;
   else if ( m_prms[0] == "fastctc"      ) m_app = casa::RunManager::fastctc;
   else if ( m_prms[0] == "fasttouch7"   ) m_app = casa::RunManager::fasttouch7;
   else if ( m_prms[0] == "fastmig"      ) m_app = casa::RunManager::fastmig;
   else if ( m_prms[0] == "track1d"      ) m_app = casa::RunManager::track1d;
   else if ( m_prms[0] == "generic"      ) m_app = casa::RunManager::generic;
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown cauldron application name: " << m_prms[0];
   }
}

void CmdAddCldApp::execute( casa::ScenarioAnalysis & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Add cauldron application to calculation pipeline " << m_prms[0] << "(";

      for ( size_t p = 1; p < m_prms.size(); ++p ) std::cout << m_prms[p] << ((p == m_prms.size() - 1) ? "" : ",");
      
      std::cout << ")" << std::endl;
   }
   casa::CauldronApp * app = 0;
   
   size_t p = 1;

   if ( casa::RunManager::generic == m_app )
   {
      int cpus = atol( m_prms[p++].c_str() );
      const std::string & appName = m_prms[p++];
      app = casa::RunManager::createApplication( casa::RunManager::generic, cpus, appName );
   }
   else { app = casa::RunManager::createApplication( static_cast<casa::RunManager::ApplicationType>( m_app ) ); }

   assert( 0 != app );

   for ( ; p < m_prms.size(); ++p ) app->addOption( m_prms[p] );

   casa::RunManager & rm = sa.runManager();
   if ( ErrorHandler::NoError != rm.addApplication( app ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }
}
