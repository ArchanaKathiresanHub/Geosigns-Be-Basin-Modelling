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
#include "CmdSetFilterOneDResults.h"
#include "casaAPI.h"
#include "LogHandler.h"

#include <iostream>


CmdSetFilterOneDResults::CmdSetFilterOneDResults( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   std::size_t size = m_prms.size();
   m_filterAlgorithm = size > 0 ? m_prms[0] : "";
   for (unsigned int i = 1; i<size; ++i)
   {
      m_excludeSet.insert(std::atoi(m_prms[i].c_str()));
   }
}

void CmdSetFilterOneDResults::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Setting a filter for selecting the parameters from 1D optimizations... ";

   if ( ErrorHandler::NoError != sa->setFilterOneDResults( m_filterAlgorithm, m_excludeSet ) )
   {
      throw ErrorHandler::Exception( sa->errorCode( ) ) << sa->errorMessage( );
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Filter set successfully";
}

void CmdSetFilterOneDResults::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <filtering algorithm> <exclude set> \n";
   std::cout << "  - This command sets a filter for selecting the parameters from 1D optimizations.\n";
   std::cout << "     Where:\n";
   std::cout << "       filtering algorithm - the number of the filtering algorithm (0 = no filter, 1 = smart selection of the lithofractions)\n";
   std::cout << "       exclude set         - the set of calibration parameters which are excluded from the 1D results\n";
}


