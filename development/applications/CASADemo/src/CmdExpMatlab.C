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
#include "CmdExpMatlab.h"
#include "MatlabExporter.h"

#include "casaAPI.h"
#include "CmdLocation.h"
#include "CmdBaseProject.h"

#include <cstdlib>
#include <iostream>

CmdExpMatlab::CmdExpMatlab( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   assert( m_prms.size() == 1 );

   m_dataFileName = m_prms[0];
   if ( m_dataFileName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty Matlab data file name for exporting results";
}

void CmdExpMatlab::execute( casa::ScenarioAnalysis & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Export CASA results to : " << m_dataFileName << std::endl;
   }

   // Go over all command and extract location and base case name
   const std::vector< SharedCmdPtr > & cmds = m_commander.cmdQueue();
   
   std::string baseCaseName;
   std::string location;

   for ( size_t i = 0; i < cmds.size() && ( baseCaseName.empty() || location.empty() ); ++i )
   {
      CmdLocation    * cmdLoc = dynamic_cast<CmdLocation*   >( cmds[i].get() );
      CmdBaseProject * cmdBP  = dynamic_cast<CmdBaseProject*>( cmds[i].get() );

      if ( cmdLoc ) location     = cmdLoc->casesLocation();
      if ( cmdBP  ) baseCaseName = cmdBP->baseProjectName();
   }

   MatlabExporter mex( m_dataFileName );
   mex.exportScenario( sa, baseCaseName, location );  
}
