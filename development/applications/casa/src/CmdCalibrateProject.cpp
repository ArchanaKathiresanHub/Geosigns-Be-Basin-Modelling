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
#include "CmdCalibrateProject.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include "utilities.h"

#include <cstdlib>
#include <iostream>

CmdCalibrateProject::CmdCalibrateProject( CasaCommander & parent, const std::vector< std::string > & cmdPrms )
                                                      : CasaCmd( parent, cmdPrms )
{
   m_bmcName   = m_prms.size() > 0 ? m_prms[0] : "";
   m_optimAlg  = m_prms.size() > 1 ? m_prms[1] : "";
   m_cldVer    = m_prms.size() > 2 ? m_prms[2] : "";
   m_transformation = m_prms.size() > 3 ? m_prms[3] : "none";
   m_relativeReduction = m_prms.size( ) > 4 ? atof( m_prms[4].c_str( ) ) : 0.05;
   m_keepHist  = m_prms.size() > 5 ? ( m_prms[5] == "KeepHistory" ? true : false ) : false;


   if ( m_bmcName.empty()  ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty output project name for project calibration";
   if ( m_optimAlg.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty optimization algorithm name was given";

   casaAppUtils::checkCauldronVersion(m_cldVer);
}

void CmdCalibrateProject::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Starting calibration loop...";

   // set run manager to use local job scheduler and the given cauldron simulator version
   casa::RunManager & rm = sa->runManager();
   if ( ErrorHandler::NoError != rm.addApplication( new casa::CauldronApp( "datadriller", false) ) )
   {
     throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   if ( ErrorHandler::NoError != rm.setCauldronVersion( m_cldVer.c_str() ) ||
        ErrorHandler::NoError != rm.setClusterName( "LOCAL" ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }

   if ( ErrorHandler::NoError != sa->calibrateProjectUsingOptimizationAlgorithm( m_bmcName, m_optimAlg, m_transformation, m_relativeReduction, m_keepHist ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Project calibration succeeded.";
}

void CmdCalibrateProject::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <calProjFileName> <algoName> [<cldVersion>] [KeepHistory] \n";
   std::cout << "  Where:\n";
   std::cout << "  calProjFileName - project file name which will keep calibrated parameter values\n";
   std::cout << "  algoName        - optimization algorithm name. Implemented optimization algorith list is following: \n";
   std::cout << "                    LM - variant of the Levenberg Marquardt algorithm implemented in Eigen.\n";
   std::cout << "  cldVersion      - (Optional) simulator version. Must be installed in IBS folder. Could be specified as \"Default\".\n";
   std::cout << "                    In this case the same simulator version as casa application will be used.\n";
   std::cout << "  log10           - (Optional) If specified, parameters will be log10-transformed in the optimization algorithm\n";
   std::cout << "  KeepHistory     - (Optional) if specified, all intermediate projects on optimization path will not be deleted\n";
   std::cout << "\n";
   std::cout << "     Here is an examples of using \"" << cmdName << "\" command:\n";
   std::cout << "         " << cmdName << " \"CalibratedCase.project3d\" \"LM\" \n";
}


