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
#include "CmdScenarioID.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include <cstdlib>
#include <iostream>

CmdScenarioID::CmdScenarioID( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_id = m_prms.size() > 0 ? m_prms[0] : "";
   if ( m_id.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Scenario ID is not provided";
}

void CmdScenarioID::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Setting scenario ID: " << m_id << "...";
   
   if ( ErrorHandler::NoError != sa->defineScenarioID( m_id.c_str() ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }
}

void CmdScenarioID::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <scenarioID> \n\n";
   std::cout << "  - defines scenario ID which will be presented in any CASA API generated file\n";
   std::cout << "\n";
   std::cout << "    Examples:\n";
   std::cout << "    " << cmdName << " \"NVG_UA_18328\"\n";
   std::cout << "\n";
}
