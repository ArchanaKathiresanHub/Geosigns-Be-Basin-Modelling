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
#include "CmdAddCldApp.h"

#include "casaAPI.h"
#include "Observable.h"
#include "ObsGridPropertyWell.h"
#include "ObsSpace.h"
#include "PrmWindow.h"
#include "RunCase.h"
#include "RunManager.h"

#include "LogHandler.h"

#include "FolderPath.h"
#include "FilePath.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>


CmdGenerateMultiOneD::CmdGenerateMultiOneD( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_cldVer            = m_prms.size() > 0 ?   m_prms[0]  : "Default";
   m_transformation    = m_prms.size() > 1 ?   m_prms[1]  : "none";
   m_relativeReduction = m_prms.size() > 2 ?   atof( m_prms[2].c_str() ) : 0.05;
   m_keepHist          = m_prms.size() > 3 ? ( m_prms[3] == "KeepHistory" ? true : false ) : false;
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

void CmdGenerateMultiOneD::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <Cauldron version> [KeepHistory] \n\n";
   std::cout << "  - generates a sert of 1D sceanrios for each well target by defining window 2x2 nodes around the well.\n";
   std::cout << "    Also generate casa scripts file to run for each 1D scenario parameters calibration\n";
   std::cout << "    Here:\n";
   std::cout << "    <Cauldron version> - simulator version which will be used to run 1D scenario. Must be installed in IBS folder.\n";
   std::cout << "                         Could be specified as \"Default\". In this case the same simulator version as casa\n";
   std::cout << "                         application will be used.\n";
   std::cout << "    KeepHistory        - (Optional) If specified, Calibration run for 1D projects will not delete intermediate project files\n";
   std::cout << "    log10              - (Optional) If log10, parameters will be log10-transformed in the optimization algorithm\n";
   std::cout << "\n";
   std::cout << "    Examples:\n";
   std::cout << "    #      Cauldron version.\n";
   std::cout << "    " << cmdName << " \"v2016.0501\"\n";
   std::cout << "\n";
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
   oss << "\ncalibrateProject \"" << sa->baseCaseProjectFileName() << "\" \"LM\" \""
                                  << m_cldVer
                                  << "\" \"" << m_transformation << "\""
                                  << " " << m_relativeReduction << " "
                                  <<  ( m_keepHist ? " \"KeepHistory\"" : "" )
                                  << "\n";

   oss << "\nsavestate \"casa_state.bin\" \"bin\"\n";

   // Go over all cases and save scenario file
   // write a casa command file for multi1D cases only if 1d RunCases exist
   sa->doeCaseSet().filterByExperimentName( "OneDProjects" );
   for ( size_t i = 0; i < sa->doeCaseSet().size(); ++i )
   {
      const casa::RunCase * cs = sa->doeCaseSet().runCase(i);
      ibs::FilePath scFile( cs->projectPath() );
      scFile.cutLast(); // remove project file name from the path
      scFile << "scenario1d.casa";

      std::ofstream ofs( scFile.path().c_str(), std::ios_base::out | std::ios_base::trunc );
      if ( ofs.fail() )
      {
        throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can't save script file: " << scFile.path();
      }

      const std::string & scrptFile = oss.str();

      ofs << scrptFile;

      ofs.close();

      // Write observables of multi1D window to the file in each Case directory

      scFile.cutLast();
      scFile << "windowObservable.txt";

      std::ofstream ofsWindowObs( scFile.path().c_str(), std::ios_base::out | std::ios_base::trunc );
      if ( ofsWindowObs.fail() )
      {
        throw ErrorHandler::Exception(ErrorHandler::IoError) << "Cannot open file for writing: " << scFile.path();
      }

      std::vector<double> prmWindowObsOrigin;
      for ( unsigned int i = 0; i<cs->parametersNumber() ; ++i)
      {
        const casa::PrmWindow* ptr = dynamic_cast<const casa::PrmWindow*>((cs->parameter(i)).get());
        if ( ptr )
        {
          prmWindowObsOrigin = ptr->observableOrigin();
          break;
        }
      }

      std::ostringstream ossWindowObsStream;
      ossWindowObsStream << std::fixed;
      ossWindowObsStream << prmWindowObsOrigin[0] << " " << prmWindowObsOrigin[1];
      std::string ossWindowObs = ossWindowObsStream.str();

      ofsWindowObs << ossWindowObs;
      ofsWindowObs.close();
   }
   sa->doeCaseSet().filterByExperimentName( "" );

   // update scenario application queue and replace the sequence of cauldron applications to casa call
   // clean all applications list by resetting run manager
   LogHandler( LogHandler::INFO_SEVERITY ) << "Replacing application list with itself casa call ... ";

   sa->resetRunManager();

   casa::RunManager & rm = sa->runManager();
   casa::CauldronApp * app = casa::RunManager::createApplication( casa::RunManager::casa, 1, 0);
   if ( ! app ) { throw ErrorHandler::Exception( ErrorHandler::MemAllocError ) << "Can't add casa application to RunManager"; }
   app->addOption( "-detailed" );
   app->addOption( "scenario1d.casa" );
   if ( ErrorHandler::NoError != rm.addApplication( app ) ) { throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage(); }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Generation of the set of 1D scenarios is completed";
}

