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


CmdGenerateThreeDFromOneD::CmdGenerateThreeDFromOneD( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) :
  CasaCmd( parent, cmdPrms ),
  m_smoothingMethod( 0 ),
  m_smoothingRadius( 0.0 ),
  m_nrOfThreads( 1 )
{
  if ( m_prms.size() > 0 )
  {
    if ( m_prms[0].find( "Gaussian", 0 ) != std::string::npos ) m_smoothingMethod = 0;
    if ( m_prms[0].find( "MovingAverage", 0 ) != std::string::npos ) m_smoothingMethod = 1;
  }
  if ( m_prms.size() > 1 )
  {
    m_smoothingRadius = std::atof( m_prms[1].c_str() );
  }
  if ( m_prms.size() > 2 )
  {
    m_nrOfThreads = std::atoi( m_prms[2].c_str() );
  }
}

void CmdGenerateThreeDFromOneD::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Making the averages and saving the 3D model ... ";

   if ( ErrorHandler::NoError != sa->generateThreeDFromOneD( "BestMatchedOneDCases", m_smoothingMethod, m_smoothingRadius, m_nrOfThreads ) )
   {
      throw ErrorHandler::Exception( sa->errorCode( ) ) << sa->errorMessage( );
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Saving of the 3D model finished";
}

void CmdGenerateThreeDFromOneD::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <filter> <radius> <threads> \n";
   std::cout << "  - Make the averages (maps for lithopercentages) and save the results in one 3D project file.\n";
   std::cout << "    For lithopercentages 2D maps for each layer are generated using the Natural Neighbour algorithm\n";
   std::cout << "    A new folder ThreeDFromOneD is created and the optimal 3D project file saved inside the ThreeDFromOneD folder. \n";
   std::cout << "  filter:     Gaussian or MovingAverage, filter used to smoothen the masp after the NN algorithm. Default Gaussian \n";
   std::cout << "  radius[m]:  Radius of influence [m] used for smoothing. Default 0.0, resulting in no smoothing \n";
   std::cout << "  threads:    Number of threads used in the smoothing step. Default 1 \n";
   std::cout << "  - Here are some examples of using \"" << cmdName << "\" command:\n";
   std::cout << "    " << cmdName << " Gaussian 1000.0 1 \n";
   std::cout << "    " << cmdName << " MovingAverage 5000.0 4 \n";
   std::cout << "    " << cmdName << " Gaussian 3000.0 \n";
}


