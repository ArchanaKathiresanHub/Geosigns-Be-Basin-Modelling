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
#include "CmdGenerateMultiOneD.h"

#include "casaAPI.h"
#include "RunCase.h"
#include "RunManager.h"

#include "LogHandler.h"

#include "FolderPath.h"
#include "FilePath.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

CmdGenerateMultiOneD::CmdGenerateMultiOneD( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
}

void CmdGenerateMultiOneD::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Extracting 1D projects for each well ... ";

   if ( ErrorHandler::NoError != sa->extractOneDProjects( "OneDProjects" ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Extraction of 1D projects finished";
}

void CmdGenerateMultiOneD::generateScenarioScripts( std::unique_ptr<casa::ScenarioAnalysis> & sa  ) const
{
   // Prepare .casa scenario to run one 1D windowed project as a string
   std::ostringstream oss;

   LogHandler( LogHandler::INFO_SEVERITY ) << "Preparing scenario command file for 1D project ... ";

   const std::vector< SharedCmdPtr> & cmdQueue = m_commander.cmdQueue();
   for ( size_t i = 0; i < cmdQueue.size() && cmdQueue[i].get() != this; ++i )
   {
      oss << m_commander.toString( cmdQueue[i].get() ) << "\n";
   }
   oss << "\nlocation \".\"\n";
   oss << "\nrun \"LOCAL\" 5\n";

   // Go over all cases and save scenario file
   // write a casa command file for multi1D cases only if 1d RunCases exist
   sa->doeCaseSet().filterByExperimentName( "OneDProjects" );
   for ( size_t i = 0; i < sa->doeCaseSet().size(); ++i )
   {
      const casa::RunCase * cs = sa->doeCaseSet()[i];
      ibs::FilePath scFile( cs->projectPath() );
      scFile.cutLast(); // remove project file name from the path
      scFile << "scenario1d.casa";

      std::ofstream ofs( scFile.path().c_str(), std::ios_base::out | std::ios_base::trunc );
      if ( !ofs.is_open() ) { throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can't save script file: " << scFile.path(); }
      ofs << oss.str();
   }
   sa->doeCaseSet().filterByExperimentName( "" );

   // update scenario application queue and replace the sequnce of cauldron applications to casa call
   // clean all applications list by resetting run mananger
   LogHandler( LogHandler::INFO_SEVERITY ) << "Replacing application list with itself casa call ... ";
   sa->resetRunManager();
   casa::RunManager & rm = sa->runManager();
   casa::CauldronApp * app = casa::RunManager::createApplication( casa::RunManager::casa, 1, 0, 
#ifdef _WIN32
                                                                  casa::CauldronApp::cmd
#else
                                                                  casa::CauldronApp::bash
#endif // _WIN32
                                                                );
   if ( ! app ) { throw ErrorHandler::Exception( ErrorHandler::MemAllocError ) << "Can't add casa application to RunManager"; }
   app->addOption( "scenario1d.casa" );
   if ( ErrorHandler::NoError != rm.addApplication( app ) ) { throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage(); }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Generation of the set of 1D scenarios is completed";
}

