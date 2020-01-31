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
  m_interpolationMethod( 0 ),
  m_IDWpower( 2.0 ),
  m_smoothingMethod( 0 ),
  m_smoothingRadius( 0.0 ),
  m_nrOfThreads( 1 )
{
  int index = 0;
  if ( m_prms.size() > index )
  {
    if ( m_prms[index].find( "IDW", 0 ) != std::string::npos )
    {
      m_interpolationMethod = 0;
      index++;
      if ( m_prms.size() > index )
      {
        m_IDWpower = std::atoi( m_prms[index].c_str() );
      }
    }
    else if ( m_prms[index].find( "NN", 0 ) != std::string::npos )
    {
      m_interpolationMethod = 1;
    }
    index++;
  }

  if ( m_prms.size() > index )
  {
    if ( m_prms[index].find( "Gaussian", 0 ) != std::string::npos ) m_smoothingMethod = 0;
    if ( m_prms[index].find( "MovingAverage", 0 ) != std::string::npos ) m_smoothingMethod = 1;
    index++;
  }
  if ( m_prms.size() > index )
  {
    m_smoothingRadius = std::atof( m_prms[index].c_str() );
    index++;
  }
  if ( m_prms.size() > index )
  {
    m_nrOfThreads = std::atoi( m_prms[index].c_str() );
    index++;
  }
}

void CmdGenerateThreeDFromOneD::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Making the averages and saving the 3D model ... ";

   if ( ErrorHandler::NoError != sa->generateThreeDFromOneD( "BestMatchedOneDCases",
                                                             m_interpolationMethod,
                                                             m_IDWpower,
                                                             m_smoothingMethod,
                                                             m_smoothingRadius,
                                                             m_nrOfThreads ) )
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
   std::cout << "  method:     IDW or NN, interpolation method: Inverse Distance Weighting (default) or Nearest Neighbor. \n";
   std::cout << "  power:      Power parameter for IDW method. power = 2.0 by default. \n";
   std::cout << "                obligation to set if the followed by the following smoothing parameters \n";
   std::cout << "  filter:     Gaussian or MovingAverage, filter used to smoothen the maps after the interpolation algorithm. Default Gaussian \n";
   std::cout << "  radius[m]:  Radius of influence [m] used for smoothing. Default 0.0, resulting in no smoothing \n";
   std::cout << "  threads:    Number of threads used in the smoothing step. Default 1 \n";
   std::cout << "  - Here are some examples of using \"" << cmdName << "\" command:\n";
   std::cout << "      default equivalence: \n";
   std::cout << "    " << cmdName << " \n";
   std::cout << "    " << cmdName << " IDW \n";
   std::cout << "    " << cmdName << " IDW 2.0 \n";
   std::cout << "      other power parameter:  \n";
   std::cout << "    " << cmdName << " IDW 1.0 \n";
   std::cout << "    " << cmdName << " IDW 4.0 \n";
   std::cout << "      variation of interpolation methods with additional smoothing: \n";
   std::cout << "    " << cmdName << " NN Gaussian 1000.0 1 \n";
   std::cout << "    " << cmdName << " NN MovingAverage 5000.0 4 \n";
   std::cout << "    " << cmdName << " IDW 2.0 Gaussian 3000.0 \n";
   std::cout << "    " << cmdName << " IDW 1.0 Gaussian 3000.0 \n";
   std::cout << "    " << cmdName << " IDW 8.0 MovingAverage 1000.0 \n";
}


