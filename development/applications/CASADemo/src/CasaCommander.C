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

#include "CasaCommander.h"
#include "CfgFileParser.h"

#include "CmdAddCldApp.h"
#include "CmdAddObs.h"
#include "CmdAddVarPrm.h"
#include "CmdBaseProject.h"
#include "CmdCreateResponse.h"
#include "CmdDoE.h"
#include "CmdEvaluateResponse.h"
#include "CmdExpMatlab.h"
#include "CmdLocation.h"
#include "CmdPlotMC.h"
#include "CmdPlotRSProxyQC.h"
#include "CmdRun.h"
#include "CmdRunMC.h"
#include "CmdSaveState.h"
#include "CmdLoadState.h"

#include <typeinfo>

CasaCommander::CasaCommander()
{
   m_msgLvl = Minimal;
}

void CasaCommander::addCommand( const std::string & cmdName, const std::vector< std::string > & prms )
{
   SharedCmdPtr  cmd;

   if (      cmdName == "app"          ) cmd.reset( new CmdAddCldApp(        *this, prms ) );// add cauldron application to pipeline
   else if ( cmdName == "base_project" ) cmd.reset( new CmdBaseProject(      *this, prms ) );// set scenario base case 
   else if ( cmdName == "varprm"       ) cmd.reset( new CmdAddVarPrm(        *this, prms ) );// create variable parameter
   else if ( cmdName == "target"       ) cmd.reset( new CmdAddObs(           *this, prms ) );// create observable
   else if ( cmdName == "doe"          ) cmd.reset( new CmdDoE(              *this, prms ) );// create doe
   else if ( cmdName == "run"          ) cmd.reset( new CmdRun(              *this, prms ) );// run planned DoE experiments
   else if ( cmdName == "location"     ) cmd.reset( new CmdLocation(         *this, prms ) );// where cases will be generated, run mutator
   else if ( cmdName == "response"     ) cmd.reset( new CmdCreateResponse(   *this, prms ) );// calculate coefficients for response surface approximation
   else if ( cmdName == "evaluate"     ) cmd.reset( new CmdEvaluateResponse( *this, prms ) );// calculate observables value using response surface approximation
   else if ( cmdName == "exportMatlab" ) cmd.reset( new CmdExpMatlab(        *this, prms ) );// export all data to matlab file
   else if ( cmdName == "montecarlo"   ) cmd.reset( new CmdRunMC(            *this, prms ) );// run MC/MCMC simulation
   else if ( cmdName == "savestate"    ) cmd.reset( new CmdSaveState(        *this, prms ) );// save CASA state to file
   else if ( cmdName == "loadstate"    ) cmd.reset( new CmdLoadState(        *this, prms ) );// load CASA state from the file
   else if ( cmdName == "plotMC"       ) cmd.reset( new CmdPlotMC(           *this, prms ) );// create plot with MC/MCMC results
   else if ( cmdName == "plotRSProxyQC") cmd.reset( new CmdPlotRSProxyQC(    *this, prms ) );// create QC plot for RSProxy results

   else throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown command: " << cmdName;

   m_cmds.push_back( cmd );

   if ( m_msgLvl > Minimal )
   {
      std::cout << "Added command to the command queue: " << typeid(*(cmd.get())).name() << "(";
      for ( size_t i = 0; i < prms.size(); ++i )
      {
         std::cout << (i == 0 ? " " : ", ") << prms[i];
      }
      std::cout << std::endl;
   }
}

void CasaCommander::executeCommands( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   for ( size_t i = 0; i < m_cmds.size(); ++i )
   {
      m_cmds[i]->execute( sa );
   }
}

void CasaCommander::printHelpPage()
{
   std::cout << "Input file for application usually has .casa extension and contains a set of commands with their parameters\n";
   std::cout << "Here is a shot description of implemented commands\n";

   CmdAddCldApp::printHelpPage( "app" );

}
