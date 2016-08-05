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
#include "CmdAddObs.h"
#include "CmdAddVarPrm.h"
#include "CmdBaseProject.h"
#include "CmdCalibrateProject.h"
#include "CmdCreateResponse.h"
#include "CmdDoE.h"
#include "CmdEvaluateResponse.h"
#include "CmdExpMatlab.h"
#include "CmdExpDataTxt.h"
#include "CmdGenerateBestMatchedCase.h"
#include "CmdLocation.h"
#include "CmdPlotMC.h"
#include "CmdPlotP10P90.h"
#include "CmdPlotPareto.h"
#include "CmdPlotRSProxyQC.h"
#include "CmdPlotTornado.h"
#include "CmdRun.h"
#include "CmdRunBaseCase.h"
#include "CmdRunReload.h"
#include "CmdRunMC.h"
#include "CmdSaveState.h"
#include "CmdScenarioID.h"
#include "CmdLoadState.h"
#include "CmdGenerateMultiOneD.h"
#include "CmdImportOneDResults.h"
#include "CmdSetFilterOneDResults.h"
#include "CmdGenerateThreeDFromOneD.h"

#include <typeinfo>

const char * CasaCommander::s_CNAddCldApp        = "app";
const char * CasaCommander::s_CNBaseProject      = "base_project"; 
const char * CasaCommander::s_CNAddObservable    = "target";
const char * CasaCommander::s_CNScenarioID       = "scenarioID";
const char * CasaCommander::s_CNAddVarPrm        = "varprm";
const char * CasaCommander::s_CNExpDataTxt       = "exportDataTxt";
const char * CasaCommander::s_CNGenerateBMCase   = "generateCalibratedCase";
const char * CasaCommander::s_CNCalibrateProject = "calibrateProject";
const char * CasaCommander::s_CNRun              = "run";
const char * CasaCommander::s_CNRunBaseCase      = "runBaseCase";
const char * CasaCommander::s_CNRunReload        = "runReload";
const char * CasaCommander::s_CNPlotMC           = "plotMC";
const char * CasaCommander::s_CNPlotP10P90       = "plotP10P90";
const char * CasaCommander::s_CNPlotPareto       = "plotPareto";
const char * CasaCommander::s_CNPlotTornado      = "plotTornado";
const char * CasaCommander::s_CNGenerateMultiOneD      = "generateMulti1D";
const char * CasaCommander::s_CNImportOneDResults      = "importOneDResults";
const char * CasaCommander::s_CNSetFilterOneDResults   = "setFilterOneDResults";
const char * CasaCommander::s_CNGenerateThreeDFromOneD = "generateThreeDFromOneD";

CasaCommander::CasaCommander()
{
   m_msgLvl = Minimal;
}

void CasaCommander::addCommand( const std::string & cmdName, const std::vector< std::string > & prms, size_t lineNum )
{
   SharedCmdPtr  cmd;

   if (      cmdName == s_CNAddCldApp        ) cmd.reset( new CmdAddCldApp(               *this, prms ) );// add cauldron application to pipeline
   else if ( cmdName == s_CNBaseProject      ) cmd.reset( new CmdBaseProject(             *this, prms ) );// set scenario base case 
   else if ( cmdName == s_CNAddVarPrm        ) cmd.reset( new CmdAddVarPrm(               *this, prms ) );// create variable parameter
   else if ( cmdName == s_CNAddObservable    ) cmd.reset( new CmdAddObs(                  *this, prms ) );// create observable
   else if ( cmdName == "doe"                ) cmd.reset( new CmdDoE(                     *this, prms ) );// create doe
   else if ( cmdName == s_CNRun              ) cmd.reset( new CmdRun(                     *this, prms ) );// run planned DoE experiments
   else if ( cmdName == s_CNRunBaseCase      ) cmd.reset( new CmdRunBaseCase(             *this, prms ) );// run base case project
   else if ( cmdName == s_CNRunReload        ) cmd.reset( new CmdRunReload(               *this, prms ) );// reload the results of run of DoE experiments
   else if ( cmdName == "location"           ) cmd.reset( new CmdLocation(                *this, prms ) );// where cases will be generated, run mutator
   else if ( cmdName == "response"           ) cmd.reset( new CmdCreateResponse(          *this, prms ) );// calculate coefficients for RS approximation
   else if ( cmdName == "evaluate"           ) cmd.reset( new CmdEvaluateResponse(        *this, prms ) );// calculate obs. value using RS approximation
   else if ( cmdName == "exportMatlab"       ) cmd.reset( new CmdExpMatlab(               *this, prms ) );// export all data to matlab file
   else if ( cmdName == s_CNExpDataTxt       ) cmd.reset( new CmdExpDataTxt(              *this, prms ) );// export data of QC-ing proxy to matlab file
   else if ( cmdName == "montecarlo"         ) cmd.reset( new CmdRunMC(                   *this, prms ) );// run MC/MCMC simulation
   else if ( cmdName == s_CNGenerateBMCase   ) cmd.reset( new CmdGenerateBestMatchedCase( *this, prms ) );// create calibrated run case
   else if ( cmdName == s_CNCalibrateProject ) cmd.reset( new CmdCalibrateProject(        *this, prms ) );// create calibrated run case using 
                                                                                                          //   global opt. alg
   else if ( cmdName == "savestate"          ) cmd.reset( new CmdSaveState(               *this, prms ) );// save CASA state to file
   else if ( cmdName == "loadstate"          ) cmd.reset( new CmdLoadState(               *this, prms ) );// load CASA state from the file
   else if ( cmdName == s_CNPlotMC           ) cmd.reset( new CmdPlotMC(                  *this, prms ) );// create plot with MC/MCMC results
   else if ( cmdName == "plotRSProxyQC"      ) cmd.reset( new CmdPlotRSProxyQC(           *this, prms ) );// create QC plot for RSProxy results
   else if ( cmdName == s_CNPlotP10P90       ) cmd.reset( new CmdPlotP10P90(              *this, prms ) );// create plot of CDF & 1-CDF for each observable
   else if ( cmdName == s_CNPlotPareto       ) cmd.reset( new CmdPlotPareto(              *this, prms ) );// create plot of Pareto diagram for parameters 
                                                                                                          //   sensitivity over all observables
   else if ( cmdName == s_CNScenarioID       ) cmd.reset( new CmdScenarioID(              *this, prms ) );// define scenario ID
   else if ( cmdName == s_CNPlotTornado      ) cmd.reset( new CmdPlotTornado(             *this, prms ) );// create Tornado diagram for each observable 
                                                                                                          //   for parameters sensitivity
   else if ( cmdName == s_CNGenerateMultiOneD ) cmd.reset( new CmdGenerateMultiOneD(      *this, prms ) );// create 1D projects for each well
   else if ( cmdName == s_CNImportOneDResults ) cmd.reset( new CmdImportOneDResults(      *this, prms ) );// import 1D results and make the averages
   else if ( cmdName == s_CNSetFilterOneDResults   ) cmd.reset( new CmdSetFilterOneDResults(   *this, prms ) );// sets the parameter filter
   else if ( cmdName == s_CNGenerateThreeDFromOneD ) cmd.reset( new CmdGenerateThreeDFromOneD( *this, prms ) );// generate the 3D case from 1D scenario
   else throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown command: " << cmdName;

   m_cmds.push_back( cmd );

   m_cmdNames.push_back( cmdName );
   m_inpFileCmdPos.push_back( lineNum );

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Added command to the command queue: " << (typeid(*(cmd.get())).name()) << "("
                              << CfgFileParser::implode( prms, "," ) << ")";
}

std::string CasaCommander::toString( const CasaCmd * pCmd )
{
   // dump command as astring to oss
   std::ostringstream oss;

   if      ( dynamic_cast<const CmdAddCldApp  *>( pCmd ) )  oss << s_CNAddCldApp;
   else if ( dynamic_cast<const CmdBaseProject*>( pCmd ) )  oss << s_CNBaseProject;
   else if ( dynamic_cast<const CmdAddVarPrm  *>( pCmd ) ) 
   { 
      const CmdAddVarPrm * cmd = dynamic_cast<const CmdAddVarPrm*>( pCmd );
      oss << s_CNAddVarPrm << " \"" << cmd->prmName() << "\""; 
   }
   else if ( dynamic_cast<const CmdAddObs*>( pCmd ) )
   {
      const CmdAddObs * cmd = dynamic_cast<const CmdAddObs*> (pCmd);
      oss << s_CNAddObservable << " \"" << cmd->obsName() << "\"";
   }
   else if ( dynamic_cast<const CmdRun                    *> (pCmd) )  oss << s_CNRun;
   else if ( dynamic_cast<const CmdRunBaseCase            *> (pCmd) )  oss << s_CNRunBaseCase;
   else if ( dynamic_cast<const CmdRunReload              *> (pCmd) )  oss << s_CNRunReload;
   else if ( dynamic_cast<const CmdLocation               *> (pCmd) )  oss << "location";
   else if ( dynamic_cast<const CmdExpMatlab              *> (pCmd) )  oss << "exportMatlab";
   else if ( dynamic_cast<const CmdExpDataTxt             *> (pCmd) )  oss << s_CNExpDataTxt;
   else if ( dynamic_cast<const CmdGenerateBestMatchedCase*> (pCmd) )  oss << s_CNGenerateBMCase;
   else if ( dynamic_cast<const CmdCalibrateProject       *> (pCmd) )  oss << s_CNCalibrateProject;
   else if ( dynamic_cast<const CmdSaveState              *> (pCmd) )  oss << "savestate";
   else if ( dynamic_cast<const CmdLoadState              *> (pCmd) )  oss << "loadstate";
   else if ( dynamic_cast<const CmdScenarioID             *> (pCmd) )  oss << s_CNScenarioID;
   else if ( dynamic_cast<const CmdGenerateMultiOneD      *> (pCmd) )  oss << s_CNGenerateMultiOneD;
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
   if ( !cmd.empty() )
   {
      std::cout << "Input file for application usually has .casa extension and contains a set of commands with their parameters\n";
      std::cout << "Here is a list of implemented commands. To get detailed help on any command just specify this command name with -help option \n";
   }
   
   if (      cmd == s_CNAddCldApp        ) { CmdAddCldApp::printHelpPage(               s_CNAddCldApp        ); }
   else if ( cmd == s_CNAddObservable    ) { CmdAddObs::printHelpPage(                  s_CNAddObservable    ); }
   else if ( cmd == s_CNAddVarPrm        ) { CmdAddVarPrm::printHelpPage(               s_CNAddVarPrm        ); }
   else if ( cmd == s_CNExpDataTxt       ) { CmdExpDataTxt::printHelpPage(              s_CNExpDataTxt       ); }
   else if ( cmd == s_CNRun              ) { CmdRun::printHelpPage(                     s_CNRun              ); }
   else if ( cmd == s_CNRunBaseCase      ) { CmdRunBaseCase::printHelpPage(             s_CNRunBaseCase      ); }
   else if ( cmd == s_CNRunReload        ) { CmdRunReload::printHelpPage(               s_CNRunReload        ); }
   else if ( cmd == s_CNGenerateBMCase   ) { CmdGenerateBestMatchedCase::printHelpPage( s_CNGenerateBMCase   ); }
   else if ( cmd == s_CNCalibrateProject ) { CmdCalibrateProject::printHelpPage(        s_CNCalibrateProject ); }
   else if ( cmd == s_CNPlotMC           ) { CmdPlotMC::printHelpPage(                  s_CNPlotMC           ); }
   else if ( cmd == s_CNPlotP10P90       ) { CmdPlotP10P90::printHelpPage(              s_CNPlotP10P90       ); }
   else if ( cmd == s_CNPlotPareto       ) { CmdPlotPareto::printHelpPage(              s_CNPlotPareto       ); }
   else if ( cmd == s_CNScenarioID       ) { CmdScenarioID::printHelpPage(              s_CNScenarioID       ); }
   else if ( cmd == s_CNPlotTornado      ) { CmdPlotTornado::printHelpPage(             s_CNPlotTornado      ); }
   else if ( cmd == s_CNGenerateMultiOneD      ) { CmdGenerateMultiOneD::printHelpPage(      s_CNGenerateMultiOneD      ); }
   else if ( cmd == s_CNImportOneDResults      ) { CmdImportOneDResults::printHelpPage(      s_CNImportOneDResults      ); }
   else if ( cmd == s_CNSetFilterOneDResults   ) { CmdSetFilterOneDResults::printHelpPage(   s_CNSetFilterOneDResults   ); }
   else if ( cmd == s_CNGenerateThreeDFromOneD ) { CmdGenerateThreeDFromOneD::printHelpPage( s_CNGenerateThreeDFromOneD ); }
   else // print all commands
   {
      std::cout << "   " << s_CNAddCldApp        << " - add new Cauldron app to application pipeline\n";
      std::cout << "   " << s_CNAddObservable    << " - specify new observable (target)\n";
      std::cout << "   " << s_CNAddVarPrm        << " - specify new variable parameter\n";
      std::cout << "   " << s_CNRun              << " - execute generated cases on HPC cluster\n";
      std::cout << "   " << s_CNRunBaseCase      << " - execute base case project on HPC cluster\n";
      std::cout << "   " << s_CNRunReload        << " - reload results of completed cases\n";
      std::cout << "   " << s_CNGenerateBMCase   << " - generate run case with parameters set from MonteCarlo simulation sample with minimal RMSE\n";
      std::cout << "   " << s_CNCalibrateProject << " - run global optimization algorithm to find parameters minimizing differences for observables value\n";

      std::cout << "   " << "\nMulti 1D SAC scenario commands:" << "\n";
      std::cout << "   " << s_CNGenerateMultiOneD << " - generate 1D projects run case set for each defined well observable \n";
      std::cout << "   " << s_CNImportOneDResults << " - import results of LM based global optimization 1D runs\n";
      std::cout << "   " << s_CNSetFilterOneDResults << " - set algrithm to filter some results for multi 1D projects\n";
      std::cout << "   " << s_CNGenerateThreeDFromOneD << " - generate multi-1d calibrated 3d project\n";

      std::cout << "   " << "\nVarious export data commands:" << "\n";
      std::cout << "   " << s_CNExpDataTxt       << " - export to text file various set of data like DoE generated parameters, observables and etc\n";

      std::cout << "   " << "\nVarious plot commands:" << "\n";
      std::cout << "   " << s_CNScenarioID       << " - define scenario ID, this ID will be copied to all CASA API generated files\n";
      std::cout << "   " << s_CNPlotMC           << " - create Matlab/Octave script to create a set of MC sampling plots for each pair of variable parameters\n";
      std::cout << "   " << s_CNPlotP10P90       << " - create Matlab/Octave script to plot P10-P90 CDF diagram for each observable\n";
      std::cout << "   " << s_CNPlotPareto       << " - create Matlab/Octave script to plot Pareto diagram for parameters sensitivity over all observables\n";
      std::cout << "   " << s_CNPlotTornado      << " - create Matlab/Octave script to plot Tornado diagrams for parameters sensitivity for each observable\n";           
   }
}


