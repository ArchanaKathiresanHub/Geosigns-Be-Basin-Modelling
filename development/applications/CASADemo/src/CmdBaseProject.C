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
#include "CmdBaseProject.h"

#include "casaAPI.h"

#include <cstdlib>
#include <iostream>

CmdBaseProject::CmdBaseProject( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   assert( m_prms.size() == 1 );

   m_baseProjectName = m_prms[0];
   if ( m_baseProjectName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty base case project name";
}

void CmdBaseProject::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Set base case: " << m_baseProjectName << std::endl;
   }

   if ( ErrorHandler::NoError != sa->defineBaseCase( m_baseProjectName.c_str() ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }
}
