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
#include "CmdLocation.h"

#include "casaAPI.h"

#include <cstdlib>
#include <iostream>

CmdLocation::CmdLocation( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_locPath = m_prms.size() > 0 ? m_prms[0] : "";
   if ( m_locPath.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty path to generated cases";
}

void CmdLocation::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Generating the set of case in folder: " << m_locPath << "..." << std::endl;
   }
   
   if ( ErrorHandler::NoError != sa->setScenarioLocation( m_locPath.c_str() )  ||
        ErrorHandler::NoError != sa->applyMutations( sa->doeCaseSet() ) ||
        ErrorHandler::NoError != sa->dataDigger().requestObservables( sa->obsSpace(), sa->doeCaseSet() )
      )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }
}
