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
#include "CmdGenerateThreeDFromOneD.h"
#include "casaAPI.h"
#include "LogHandler.h"

#include <iostream>


CmdGenerateThreeDFromOneD::CmdGenerateThreeDFromOneD( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
}

void CmdGenerateThreeDFromOneD::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Making the averages and saving the 3D model ... ";

   if ( ErrorHandler::NoError != sa->generateThreeDFromOneD( "BestMatchedOneDCases" ) )
   {
      throw ErrorHandler::Exception( sa->errorCode( ) ) << sa->errorMessage( );
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Saving of the 3D model finished";
}

void CmdGenerateThreeDFromOneD::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << "\n";
   std::cout << "  - Make the averages (maps for lithopercentages) and save the results in one 3D project file.\n";
   std::cout << "    For lithopercentages 2D maps for each layer are generated using the Natural Neighbour algorithm\n";
   std::cout << "    A new folder ThreeDFromOneD is created and the optimal 3D project file saved inside the ThreeDFromOneD folder. \n";
}


