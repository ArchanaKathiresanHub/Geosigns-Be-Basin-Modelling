//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CasaCommander.h"
#include "CmdLocation.h"
#include "CmdGenerateMultiOneD.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include <cstdlib>
#include <iostream>

CmdLocation::CmdLocation( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_locPath = m_prms.size() > 0 ? m_prms[0] : "";
   if ( m_locPath.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty path to generated cases";

   for  ( int i = 1; i < m_prms.size(); ++i )
   {
     if ( m_prms[i] == "append" ) m_appendCases = true;
     if ( m_prms[i] == "noOptimization" ) m_noOptimization = true;
   }
}

void CmdLocation::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Generating the set of cases in folder: " << m_locPath << "...";
   if ( ErrorHandler::NoError != sa->setScenarioLocation( m_locPath.c_str(), m_appendCases ) ||
        ErrorHandler::NoError != sa->applyMutations(      sa->doeCaseSet() , m_appendCases )
      )
   {
      throw ErrorHandler::Exception(sa->errorCode()) << sa->errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Validating generated cases...";
   if ( ErrorHandler::NoError != sa->validateCaseSet(     sa->doeCaseSet()  ) )
   {
      throw ErrorHandler::Exception(sa->errorCode()) << sa->errorMessage();
   }

   if ( !m_noOptimization )
   {
     // Generate 1D .casa scenario files for each of 1D project in case of Multi1D SAC
     const std::vector< SharedCmdPtr> & cmdq = m_commander.cmdQueue();
     for ( size_t i = 0; i < cmdq.size(); ++i )
     {
        const CmdGenerateMultiOneD * cmd = dynamic_cast<const CmdGenerateMultiOneD *>( cmdq[i].get() );
        if ( cmd ) { cmd->generateScenarioScripts( sa ); break; }
     }
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "DoE cases generation succeeded";
}

void CmdLocation::printHelpPage(const char* cmdName)
{
  std::cout << "  " << cmdName << " <Location> [<AppendFlag>] \n\n"
            << "    Where:\n"
            << "       Location      - Folder name in the run folder where the run cases are stored\n"
            << "       AppendFlag    - [optional] When set to \"append\", new cases will be added after the existing run cases\n"
            << std::endl;
}


