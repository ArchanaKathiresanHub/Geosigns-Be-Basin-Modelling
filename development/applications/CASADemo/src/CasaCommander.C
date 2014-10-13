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
#include "CmdRun.h"

#include <typeinfo>

CasaCommander::CasaCommander()
{
   m_msgLvl = Minimal;
}

void CasaCommander::addCommand( CfgFileParser::CfgCommand cmdID, const std::vector< std::string > & prms )
{
   SharedCmdPtr cmd;
   switch ( cmdID )
   {
   case CfgFileParser::app:          cmd.reset( new CmdAddCldApp(        *this, prms ) ); break;
   case CfgFileParser::base_project: cmd.reset( new CmdBaseProject(      *this, prms ) ); break;
   case CfgFileParser::varprm:       cmd.reset( new CmdAddVarPrm(        *this, prms ) ); break;
   case CfgFileParser::target:       cmd.reset( new CmdAddObs(           *this, prms ) ); break;
   case CfgFileParser::doe:          cmd.reset( new CmdDoE(              *this, prms ) ); break;
   case CfgFileParser::location:     cmd.reset( new CmdLocation(         *this, prms ) ); break;
   case CfgFileParser::run:          cmd.reset( new CmdRun(              *this, prms ) ); break;
   case CfgFileParser::response:     cmd.reset( new CmdCreateResponse(   *this, prms ) ); break;
   case CfgFileParser::evaluate:     cmd.reset( new CmdEvaluateResponse( *this, prms ) ); break;
   case CfgFileParser::exportMatlab: cmd.reset( new CmdExpMatlab(        *this, prms ) ); break;
   default: throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknowd command ID: " << cmdID;  break;
   }
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

void CasaCommander::executeCommands( casa::ScenarioAnalysis & sa )
{
   for ( size_t i = 0; i < m_cmds.size(); ++i )
   {
      m_cmds[i]->execute( sa );
   }
}
