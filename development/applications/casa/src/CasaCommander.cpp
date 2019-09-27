//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ErrorHandler.h"
#include "LogHandler.h"

#include "CasaCommander.h"
#include "CfgFileParser.h"

#include "CmdAddCldApp.h"
#include "CmdAddDesignPoint.h"
#include "CmdAddObs.h"
#include "CmdAddVarPrm.h"
#include "CmdBaseProject.h"
#include "CmdCalibrateProject.h"
#include "CmdCreateResponse.h"
#include "CmdDoE.h"
#include "CmdExpDataTxt.h"
#include "CmdGenerateBestMatchedCase.h"
#include "CmdLocation.h"
#include "CmdRun.h"
#include "CmdRunBaseCase.h"
#include "CmdRunDataDigger.h"
#include "CmdRunMC.h"
#include "CmdRunPipeline.h"
#include "CmdSaveState.h"
#include "CmdScenarioID.h"
#include "CmdLoadState.h"
#include "CmdGenerateMultiOneD.h"
#include "CmdImportOneDResults.h"
#include "CmdSetFilterOneDResults.h"
#include "CmdGenerateThreeDFromOneD.h"

#include <typeinfo>

const char * CasaCommander::nameAddCldApp        = "app";
const char * CasaCommander::nameAddDesignPoint   = "addDesignPoint";
const char * CasaCommander::nameAddObservable    = "target";
const char * CasaCommander::nameAddVarPrm        = "varprm";
const char * CasaCommander::nameBaseProject      = "base_project";
const char * CasaCommander::nameCalibrateProject = "calibrateProject";
const char * CasaCommander::nameExpDataTxt       = "exportDataTxt";
const char * CasaCommander::nameGenerateBestMatchedCase = "generateCalibratedCase";
const char * CasaCommander::nameGenerateMultiOneD       = "generateMulti1D";
const char * CasaCommander::nameImportOneDResults       = "importOneDResults";
const char * CasaCommander::nameLocation         = "location";
const char * CasaCommander::nameRun              = "run";
const char * CasaCommander::nameRunBaseCase      = "runBaseCase";
const char * CasaCommander::nameRunDataDigger    = "runDataDigger";
const char * CasaCommander::nameRunPipeline      = "runPipeline";
const char * CasaCommander::nameScenarioID       = "scenarioID";
const char * CasaCommander::nameSetFilterOneDResults = "setFilterOneDResults";
const char * CasaCommander::nameSetOneThreeDFromOneD = "generateThreeDFromOneD";

CasaCommander::CasaCommander()
{
   m_msgLvl = Minimal;
}

void CasaCommander::addCommand( const std::string & cmdName, const std::vector< std::string > & prms, size_t lineNum )
{
   SharedCmdPtr  cmd;

   if (      cmdName == nameAddCldApp        ) cmd.reset( new CmdAddCldApp(               *this, prms ) );// add cauldron application to pipeline
   else if ( cmdName == nameAddDesignPoint   ) cmd.reset( new CmdAddDesignPoint(          *this, prms ) );// add a signle design point
   else if ( cmdName == nameBaseProject      ) cmd.reset( new CmdBaseProject(             *this, prms ) );// set scenario base case
   else if ( cmdName == nameAddVarPrm        ) cmd.reset( new CmdAddVarPrm(               *this, prms ) );// create variable parameter
   else if ( cmdName == nameAddObservable    ) cmd.reset( new CmdAddObs(                  *this, prms ) );// create observable
   else if ( cmdName == "doe"                ) cmd.reset( new CmdDoE(                     *this, prms ) );// create doe
   else if ( cmdName == nameRun              ) cmd.reset( new CmdRun(                     *this, prms ) );// run planned DoE experiments
   else if ( cmdName == nameRunBaseCase      ) cmd.reset( new CmdRunBaseCase(             *this, prms ) );// run base case project
   else if ( cmdName == nameRunDataDigger    ) cmd.reset( new CmdRunDataDigger(           *this, prms ) );// run datadigger on planned DoE experiments
   else if ( cmdName == nameRunPipeline      ) cmd.reset( new CmdRunPipeline(             *this, prms ) );// run the command pipeline
   else if ( cmdName == nameLocation         ) cmd.reset( new CmdLocation(                *this, prms ) );// where cases will be generated, run mutator
   else if ( cmdName == "response"           ) cmd.reset( new CmdCreateResponse(          *this, prms ) );// calculate coefficients for RS approximation
   else if ( cmdName == nameExpDataTxt       ) cmd.reset( new CmdExpDataTxt(              *this, prms ) );// export data of QC-ing proxy to matlab file
   else if ( cmdName == "montecarlo"         ) cmd.reset( new CmdRunMC(                   *this, prms ) );// run MC/MCMC simulation
   else if ( cmdName == nameGenerateBestMatchedCase   ) cmd.reset( new CmdGenerateBestMatchedCase( *this, prms ) );// create calibrated run case
   else if ( cmdName == nameCalibrateProject ) cmd.reset( new CmdCalibrateProject(        *this, prms ) );// create calibrated run case using
                                                                                                          //   global opt. alg
   else if ( cmdName == "savestate"          ) cmd.reset( new CmdSaveState(               *this, prms ) );// save CASA state to file
   else if ( cmdName == "loadstate"          ) cmd.reset( new CmdLoadState(               *this, prms ) );// load CASA state from the file
                                                                                                          //   sensitivity over all observables
   else if ( cmdName == nameScenarioID       ) cmd.reset( new CmdScenarioID(              *this, prms ) );// define scenario ID
                                                                                                          //   for parameters sensitivity
   else if ( cmdName == nameGenerateMultiOneD ) cmd.reset( new CmdGenerateMultiOneD(      *this, prms ) );// create 1D projects for each well
   else if ( cmdName == nameImportOneDResults ) cmd.reset( new CmdImportOneDResults(      *this, prms ) );// import 1D results and make the averages
   else if ( cmdName == nameSetFilterOneDResults   ) cmd.reset( new CmdSetFilterOneDResults(   *this, prms ) );// sets the parameter filter
   else if ( cmdName == nameSetOneThreeDFromOneD ) cmd.reset( new CmdGenerateThreeDFromOneD( *this, prms ) );// generate the 3D case from 1D scenario
   else throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown command: " << cmdName;

   m_cmds.push_back( cmd );

   m_cmdNames.push_back( cmdName );
   m_inpFileCmdPos.push_back( lineNum );

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Added command to the command queue: " << ( typeid( *cmd ).name() ) << "("
                              << CfgFileParser::implode( prms, "," ) << ")";
}

std::string CasaCommander::toString( const CasaCmd * pCmd )
{
   // dump command as astring to oss
   std::ostringstream oss;

   if      ( dynamic_cast<const CmdAddCldApp  *>( pCmd ) )  oss << nameAddCldApp;
   else if ( dynamic_cast<const CmdBaseProject*>( pCmd ) )  oss << nameBaseProject;
   else if ( dynamic_cast<const CmdAddVarPrm  *>( pCmd ) )  oss << nameAddVarPrm;
   else if ( dynamic_cast<const CmdAddObs*>( pCmd ) )
   {
      const CmdAddObs * cmd = dynamic_cast<const CmdAddObs*> (pCmd);
      oss << nameAddObservable << " \"" << cmd->obsName() << "\"";
   }
   else if ( dynamic_cast<const CmdRun                    *> (pCmd) )  oss << nameRun;
   else if ( dynamic_cast<const CmdRunBaseCase            *> (pCmd) )  oss << nameRunBaseCase;
   else if ( dynamic_cast<const CmdLocation               *> (pCmd) )  oss << "location";
   else if ( dynamic_cast<const CmdExpDataTxt             *> (pCmd) )  oss << nameExpDataTxt;
   else if ( dynamic_cast<const CmdGenerateBestMatchedCase*> (pCmd) )  oss << nameGenerateBestMatchedCase;
   else if ( dynamic_cast<const CmdCalibrateProject       *> (pCmd) )  oss << nameCalibrateProject;
   else if ( dynamic_cast<const CmdSaveState              *> (pCmd) )  oss << "savestate";
   else if ( dynamic_cast<const CmdLoadState              *> (pCmd) )  oss << "loadstate";
   else if ( dynamic_cast<const CmdScenarioID             *> (pCmd) )  oss << nameScenarioID;
   else if ( dynamic_cast<const CmdGenerateMultiOneD      *> (pCmd) )  oss << nameGenerateMultiOneD;
   else
   {
      return oss.str(); //return an empty string if the command is not valid
   }

   oss << " ";
   const std::vector<std::string> & prms = pCmd->cmdParameters();
   for ( size_t i = 0; i < prms.size(); ++i )
   {
      std::string prm = prms[i];
      if ( isdigit( prm[0] ) || prm[0] == '[' || prm[0] == '.' ) { oss << prm << " ";           }
      else                                                       { oss << "\"" << prm << "\" "; }
   }

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "The command " << ( typeid( *(pCmd)).name() ) << " was converted to string\n";

   return oss.str();
}

void CasaCommander::executeCommands( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   for ( size_t i = 0; i < m_cmds.size(); ++i )
   {
      m_curCmd = i;
      m_cmds[i]->execute( sa );
   }
}

void CasaCommander::printHelpPage( const std::string & cmd )
{
   if (      cmd == nameAddCldApp        ) { CmdAddCldApp::printHelpPage(               nameAddCldApp        ); }
   else if ( cmd == nameAddDesignPoint   ) { CmdAddDesignPoint::printHelpPage(          nameAddDesignPoint   ); }
   else if ( cmd == nameAddObservable    ) { CmdAddObs::printHelpPage(                  nameAddObservable    ); }
   else if ( cmd == nameAddVarPrm        ) { CmdAddVarPrm::printHelpPage(               nameAddVarPrm        ); }
   else if ( cmd == nameLocation         ) { CmdLocation::printHelpPage(                nameLocation         ); }
   else if ( cmd == nameExpDataTxt       ) { CmdExpDataTxt::printHelpPage(              nameExpDataTxt       ); }
   else if ( cmd == nameRun              ) { CmdRun::printHelpPage(                     nameRun              ); }
   else if ( cmd == nameRunBaseCase      ) { CmdRunBaseCase::printHelpPage(             nameRunBaseCase      ); }
   else if ( cmd == nameRunDataDigger    ) { CmdRunDataDigger::printHelpPage(           nameRunDataDigger    ); }
   else if ( cmd == nameGenerateBestMatchedCase   ) { CmdGenerateBestMatchedCase::printHelpPage( nameGenerateBestMatchedCase   ); }
   else if ( cmd == nameCalibrateProject ) { CmdCalibrateProject::printHelpPage(        nameCalibrateProject ); }
   else if ( cmd == nameScenarioID       ) { CmdScenarioID::printHelpPage(              nameScenarioID       ); }
   else if ( cmd == nameGenerateMultiOneD      ) { CmdGenerateMultiOneD::printHelpPage(      nameGenerateMultiOneD      ); }
   else if ( cmd == nameImportOneDResults      ) { CmdImportOneDResults::printHelpPage(      nameImportOneDResults      ); }
   else if ( cmd == nameSetFilterOneDResults   ) { CmdSetFilterOneDResults::printHelpPage(   nameSetFilterOneDResults   ); }
   else if ( cmd == nameSetOneThreeDFromOneD ) { CmdGenerateThreeDFromOneD::printHelpPage( nameSetOneThreeDFromOneD ); }
   else // print all commands
   {
     std::cout << "Input file for application usually has .casa extension and contains a set of commands with their parameters\n";
     std::cout << "Here is a list of implemented commands. To get detailed help on any command just specify this command name with -help option \n";
     std::cout << "   " << nameAddCldApp        << " - add new Cauldron app to application pipeline\n";
     std::cout << "   " << nameAddObservable    << " - specify new observable (target)\n";
     std::cout << "   " << nameAddVarPrm        << " - specify new variable parameter\n";
     std::cout << "   " << nameAddDesignPoint   << " - add a design point to the list of experiments\n";
     std::cout << "   " << nameRun              << " - execute generated cases on HPC cluster\n";
     std::cout << "   " << nameRunBaseCase      << " - execute base case project on HPC cluster\n";
     std::cout << "   " << nameRunDataDigger    << " - execute the data digger to obtain observable data\n";
     std::cout << "   " << nameLocation         << " - define the run location\n";
     std::cout << "   " << nameGenerateBestMatchedCase   << " - generate run case with parameters set from MonteCarlo simulation sample with minimal RMSE\n";
     std::cout << "   " << nameCalibrateProject << " - run global optimization algorithm to find parameters minimizing differences for observables value\n";

     std::cout << "   " << "\nMulti 1D SAC scenario commands:" << "\n";
     std::cout << "   " << nameGenerateMultiOneD << " - generate 1D projects run case set for each defined well observable \n";
     std::cout << "   " << nameImportOneDResults << " - import results of LM based global optimization 1D runs\n";
     std::cout << "   " << nameSetFilterOneDResults << " - set algrithm to filter some results for multi 1D projects\n";
     std::cout << "   " << nameSetOneThreeDFromOneD << " - generate multi-1d calibrated 3d project\n";

     std::cout << "   " << "\nExport data command:" << "\n";
     std::cout << "   " << nameExpDataTxt       << " - export to text file various set of data like DoE generated parameters, observables and etc\n";

     std::cout << "   " << nameScenarioID       << " - define scenario ID, this ID will be copied to all CASA API generated files\n";
   }
}


