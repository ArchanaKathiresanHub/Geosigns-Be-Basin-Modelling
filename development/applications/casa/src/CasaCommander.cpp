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

#include <typeinfo>

static const char * CNAddCldApp        = "app";
static const char * CNAddObservable    = "target";
static const char * CNScenarioID       = "scenarioID";
static const char * CNAddVarPrm        = "varprm";
static const char * CNExpDataTxt       = "exportDataTxt";
static const char * CNGenerateBMCase   = "generateCalibratedCase";
static const char * CNCalibrateProject = "calibrateProject";
static const char * CNRun              = "run";
static const char * CNRunBaseCase      = "runBaseCase";
static const char * CNRunReload        = "runReload";
static const char * CNPlotMC           = "plotMC";
static const char * CNPlotP10P90       = "plotP10P90";
static const char * CNPlotPareto       = "plotPareto";
static const char * CNPlotTornado      = "plotTornado";
static const char * CNGenerateMultiOneD= "generateMulti1D";
static const char * CNImportOneDResults= "importOneDResults";

CasaCommander::CasaCommander()
{
   m_msgLvl = Minimal;
}

void CasaCommander::addCommand( const std::string & cmdName, const std::vector< std::string > & prms, size_t lineNum )
{
   SharedCmdPtr  cmd;

   if (      cmdName == CNAddCldApp        ) cmd.reset( new CmdAddCldApp(               *this, prms ) );// add cauldron application to pipeline
   else if ( cmdName == "base_project"     ) cmd.reset( new CmdBaseProject(             *this, prms ) );// set scenario base case 
   else if ( cmdName == CNAddVarPrm        ) cmd.reset( new CmdAddVarPrm(               *this, prms ) );// create variable parameter
   else if ( cmdName == CNAddObservable    ) cmd.reset( new CmdAddObs(                  *this, prms ) );// create observable
   else if ( cmdName == "doe"              ) cmd.reset( new CmdDoE(                     *this, prms ) );// create doe
   else if ( cmdName == CNRun              ) cmd.reset( new CmdRun(                     *this, prms ) );// run planned DoE experiments
   else if ( cmdName == CNRunBaseCase      ) cmd.reset( new CmdRunBaseCase(             *this, prms ) );// run base case project
   else if ( cmdName == CNRunReload        ) cmd.reset( new CmdRunReload(               *this, prms ) );// reload the results of run of DoE experiments
   else if ( cmdName == "location"         ) cmd.reset( new CmdLocation(                *this, prms ) );// where cases will be generated, run mutator
   else if ( cmdName == "response"         ) cmd.reset( new CmdCreateResponse(          *this, prms ) );// calculate coefficients for RS approximation
   else if ( cmdName == "evaluate"         ) cmd.reset( new CmdEvaluateResponse(        *this, prms ) );// calculate obs. value using RS approximation
   else if ( cmdName == "exportMatlab"     ) cmd.reset( new CmdExpMatlab(               *this, prms ) );// export all data to matlab file
   else if ( cmdName == CNExpDataTxt       ) cmd.reset( new CmdExpDataTxt(              *this, prms ) );// export data of QC-ing proxy to matlab file
   else if ( cmdName == "montecarlo"       ) cmd.reset( new CmdRunMC(                   *this, prms ) );// run MC/MCMC simulation
   else if ( cmdName == CNGenerateBMCase   ) cmd.reset( new CmdGenerateBestMatchedCase( *this, prms ) );// create calibrated run case
   else if ( cmdName == CNCalibrateProject ) cmd.reset( new CmdCalibrateProject(        *this, prms ) );// create calibrated run case using global opt. alg
   else if ( cmdName == "savestate"        ) cmd.reset( new CmdSaveState(               *this, prms ) );// save CASA state to file
   else if ( cmdName == "loadstate"        ) cmd.reset( new CmdLoadState(               *this, prms ) );// load CASA state from the file
   else if ( cmdName == CNPlotMC           ) cmd.reset( new CmdPlotMC(                  *this, prms ) );// create plot with MC/MCMC results
   else if ( cmdName == "plotRSProxyQC"    ) cmd.reset( new CmdPlotRSProxyQC(           *this, prms ) );// create QC plot for RSProxy results
   else if ( cmdName == CNPlotP10P90       ) cmd.reset( new CmdPlotP10P90(              *this, prms ) );// create plot of CDF & 1-CDF for each observable
   else if ( cmdName == CNPlotPareto       ) cmd.reset( new CmdPlotPareto(              *this, prms ) );// create plot of Pareto diagram for parameters 
                                                                                                        // sensitivity over all observables
   else if ( cmdName == CNScenarioID       ) cmd.reset( new CmdScenarioID(              *this, prms ) );// define scenario ID
   else if ( cmdName == CNPlotTornado      ) cmd.reset( new CmdPlotTornado(             *this, prms ) );// create Tornado diagram for each observable 
                                                                                                        // for parameters sensitivity
   else if (cmdName == CNGenerateMultiOneD ) cmd.reset(new CmdGenerateMultiOneD(        *this, prms ) );// create 1D projects for each well
   else if ( cmdName == CNImportOneDResults ) cmd.reset( new CmdImportOneDResults(      *this, prms ) );// import 1D results and make the averages
   // for parameters sensitivity

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

   if      ( dynamic_cast<const CmdAddCldApp  *>( pCmd ) )  oss << CNAddCldApp   ;
   else if ( dynamic_cast<const CmdBaseProject*>( pCmd ) )  oss << "base_project";
   else if ( dynamic_cast<const CmdAddVarPrm  *>( pCmd ) ) 
   { 
      const CmdAddVarPrm * cmd = dynamic_cast<const CmdAddVarPrm*>( pCmd );
      oss << CNAddVarPrm << " \"" << cmd->prmName() << "\""; 
   }
   else if ( dynamic_cast<const CmdAddObs*>( pCmd ) )
   {
      const CmdAddObs * cmd = dynamic_cast<const CmdAddObs*> (pCmd);
      oss << CNAddObservable << " \"" << cmd->obsName() << "\"";
   }
   else if ( dynamic_cast<const CmdRun                    *> (pCmd) )  oss << CNRun;
   else if ( dynamic_cast<const CmdRunBaseCase            *> (pCmd) )  oss << CNRunBaseCase;
   else if ( dynamic_cast<const CmdRunReload              *> (pCmd) )  oss << CNRunReload;
   else if ( dynamic_cast<const CmdLocation               *> (pCmd) )  oss << "location";
   else if ( dynamic_cast<const CmdExpMatlab              *> (pCmd) )  oss << "exportMatlab";
   else if ( dynamic_cast<const CmdExpDataTxt             *> (pCmd) )  oss << CNExpDataTxt;
   else if ( dynamic_cast<const CmdGenerateBestMatchedCase*> (pCmd) )  oss << CNGenerateBMCase;
   else if ( dynamic_cast<const CmdCalibrateProject       *> (pCmd) )  oss << CNCalibrateProject;
   else if ( dynamic_cast<const CmdSaveState              *> (pCmd) )  oss << "savestate";
   else if ( dynamic_cast<const CmdLoadState              *> (pCmd) )  oss << "loadstate";
   else if ( dynamic_cast<const CmdScenarioID             *> (pCmd) )  oss << CNScenarioID;
   else if ( dynamic_cast<const CmdGenerateMultiOneD      *> (pCmd) )  oss << CNGenerateMultiOneD;
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

   LogHandler(LogHandler::DEBUG_SEVERITY) << "The command " << ( typeid( *(pCmd)).name() ) << " was converted to string\n";

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
   
   if (      cmd == CNAddCldApp        ) { CmdAddCldApp::printHelpPage(               CNAddCldApp        ); }
   else if ( cmd == CNAddObservable    ) { CmdAddObs::printHelpPage(                  CNAddObservable    ); }
   else if ( cmd == CNAddVarPrm        ) { CmdAddVarPrm::printHelpPage(               CNAddVarPrm        ); }
   else if ( cmd == CNExpDataTxt       ) { CmdExpDataTxt::printHelpPage(              CNExpDataTxt       ); }
   else if ( cmd == CNRun              ) { CmdRun::printHelpPage(                     CNRun              ); }
   else if ( cmd == CNRunBaseCase      ) { CmdRunBaseCase::printHelpPage(             CNRunBaseCase      ); }
   else if ( cmd == CNRunReload        ) { CmdRunReload::printHelpPage(               CNRunReload        ); }
   else if ( cmd == CNGenerateBMCase   ) { CmdGenerateBestMatchedCase::printHelpPage( CNGenerateBMCase   ); }
   else if ( cmd == CNCalibrateProject ) { CmdCalibrateProject::printHelpPage(        CNCalibrateProject ); }
   else if ( cmd == CNPlotMC           ) { CmdPlotMC::printHelpPage(                  CNPlotMC           ); }
   else if ( cmd == CNPlotP10P90       ) { CmdPlotP10P90::printHelpPage(              CNPlotP10P90       ); }
   else if ( cmd == CNPlotPareto       ) { CmdPlotPareto::printHelpPage(              CNPlotPareto       ); }
   else if ( cmd == CNScenarioID       ) { CmdScenarioID::printHelpPage(              CNScenarioID       ); }
   else if ( cmd == CNPlotTornado      ) { CmdPlotTornado::printHelpPage(             CNPlotTornado      ); }
   else if ( cmd == CNGenerateMultiOneD) { CmdGenerateMultiOneD::printHelpPage(       CNGenerateMultiOneD); }
   else if ( cmd == CNImportOneDResults ) { CmdImportOneDResults::printHelpPage(      CNImportOneDResults); }
   else // print all commands
   {
      std::cout << "   " << CNAddCldApp        << " - add new Cauldron app to application pipeline\n";
      std::cout << "   " << CNAddObservable    << " - specify new observable (target)\n";
      std::cout << "   " << CNAddVarPrm        << " - specify new variable parameter\n";
      std::cout << "   " << CNRun              << " - execute generated cases on HPC cluster\n";
      std::cout << "   " << CNRunBaseCase      << " - execute base case project on HPC cluster\n";
      std::cout << "   " << CNRunReload        << " - reload results of completed cases\n";
      std::cout << "   " << CNGenerateBMCase   << " - generate run case with parameters set from MonteCarlo simulation sample with minimal RMSE\n";
      std::cout << "   " << CNCalibrateProject << " - run global optimization algorithm to find parameters minimizing differences for observables value\n";

      std::cout << "   " << "\nVarious export data commands:" << "\n";
      std::cout << "   " << CNExpDataTxt       << " - export to text file various set of data like DoE generated parameters, observables and etc\n";

      std::cout << "   " << "\nVarious plot commands:" << "\n";
      std::cout << "   " << CNScenarioID       << " - define scenario ID, this ID will be copied to all CASA API generated files\n";
      std::cout << "   " << CNPlotMC           << " - create Matlab/Octave script to create a set of MC sampling plots for each pair of variable parameters\n";
      std::cout << "   " << CNPlotP10P90       << " - create Matlab/Octave script to plot P10-P90 CDF diagram for each observable\n";
      std::cout << "   " << CNPlotPareto       << " - create Matlab/Octave script to plot Pareto diagram for parameters sensitivity over all observables\n";
      std::cout << "   " << CNPlotTornado      << " - create Matlab/Octave script to plot Tornado diagrams for parameters sensitivity for each observable\n";
   }
}


