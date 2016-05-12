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
   // no members yet
}

void CmdImportOneDResults::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Importing 1D results and saving the averages in the 3D model ... ";

   if ( ErrorHandler::NoError != sa->importOneDResults( "OneDProjects" ) )
   {
      throw ErrorHandler::Exception( sa->errorCode( ) ) << sa->errorMessage( );
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Importing of the 1D results and saving the averages in the 3D model finished";
}

void CmdImportOneDResults::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << "\n";
   std::cout << "  - Collect the results from multiple 1D calibration, make the averages (maps for lithopercentages) and save the results in one 3D project file.\n";
   std::cout << "    For lithopercentages 2D maps for each layer are generated using the Natural Neighbour algorithm\n";
   std::cout << "    A new folder ThreeDFromOneD is created and the optimal 3D project file is saved there. \n";
}


