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
#include "CmdImportOneDResults.h"
#include "casaAPI.h"
#include "LogHandler.h"

#include <iostream>


CmdImportOneDResults::CmdImportOneDResults( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
}

void CmdImportOneDResults::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Importing 1D results... ";

   if ( ErrorHandler::NoError != sa->importOneDResults( "OneDProjects") )
   {
      throw ErrorHandler::Exception( sa->errorCode( ) ) << sa->errorMessage( );
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Importing of the 1D results finished";
}

void CmdImportOneDResults::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << "\n";
   std::cout << "  - Collect the results from multiple 1D calibration.\n";
}


