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
   m_dataFileName = m_prms.size() < 1 ? "casa_data.m" : m_prms[0];
   if ( m_dataFileName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty Matlab data file name for exporting results";
}

void CmdExpMatlab::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Export CASA results to : " << m_dataFileName << std::endl;
   }

   MatlabExporter mex( m_dataFileName );
   mex.exportScenario( *sa.get(), sa->baseCaseProjectFileName(), sa->scenarioLocation() );
}