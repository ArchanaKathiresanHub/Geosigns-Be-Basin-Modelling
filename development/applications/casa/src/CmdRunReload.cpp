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
#include "CmdRunReload.h"

#include "casaAPI.h"
#include "RunCase.h"

#include <cstdlib>
#include <iostream>


static void PrintObsValues( casa::ScenarioAnalysis & sc )
{
   casa::ObsSpace   & obSpace = sc.obsSpace();
   casa::RunCaseSet & rcSet = sc.doeCaseSet();

   for ( size_t rc = 0; rc < rcSet.size(); ++rc )
   {
      // go through all run cases
      const casa::RunCase * cs = rcSet[rc];

      if ( !cs ) continue;

      std::cout << "    " << cs->projectPath() << std::endl;
      std::cout << "    Observable values:" << std::endl;

      for ( size_t i = 0; i < cs->observablesNumber(); ++i )
      {
         casa::ObsValue * ov = cs->obsValue( i );
         if ( ov && ov->observable() && ov->isDouble() )
         {
            const std::vector<double>      & vals  = ov->asDoubleArray();
            const std::vector<std::string> & names = ov->observable()->name();

            for ( size_t i = 0; i < vals.size(); ++i )
            {
               std::cout << "      " << names[i] << " = " << vals[i] << "\n";
            }
         }
      }
   }
}


CmdRunReload::CmdRunReload( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_locPath = m_prms.size() > 0 ? m_prms[0] : "";
   if ( m_locPath.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty path to completed cases";
}

void CmdRunReload::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Loading completed jobs ... ";
      std::cout.flush();
   }

   if ( ErrorHandler::NoError != sa->restoreScenarioLocation( m_locPath.c_str() ) ) 
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }
 
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "done! " << std::endl;
      std::cout << "Extracting observables values ... ";
   }
  
   // collect observables value
   if ( ErrorHandler::NoError != sa->dataDigger().collectRunResults( sa->obsSpace(), sa->doeCaseSet() ) )
   {
      throw ErrorHandler::Exception( sa->dataDigger().errorCode() ) << sa->dataDigger().errorMessage();
   }

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "done! " << std::endl;
   }

   if ( m_commander.verboseLevel() > CasaCommander::Minimal ) { PrintObsValues( *sa.get() ); }
}


void CmdRunReload::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << "<Path to cases set folder>\n";
   std::cout << "  - This command allows to reload observables values from completed set of cases run\n";
   std::cout << "    The state of scenario after this command is exactly the same as after \"location\" and \"run\" commands\n";
   std::cout << "\n";
   std::cout << "    Examples:\n";
   std::cout << "    " << cmdName << " \"./CaseSet\"\n";
}
