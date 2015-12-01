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
#include "CmdGenerateBestMatchedCase.h"

#include "casaAPI.h"

#include <cstdlib>
#include <iostream>

CmdGenerateBestMatchedCase::CmdGenerateBestMatchedCase( CasaCommander & parent, const std::vector< std::string > & cmdPrms )
                                                      : CasaCmd( parent, cmdPrms )
{
   m_bmcName   = m_prms.size() > 0 ? m_prms[0]                 : "";
   m_sampleNum = m_prms.size() > 1 ? atol( m_prms[1].c_str() ) : 1;

   if ( m_bmcName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Empty project name for Best Matched Case generation";
   if ( m_sampleNum < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameter value: " << 
          m_sampleNum << ", Monte Carlo sampling numbering starts with 1";
   }
}


void CmdGenerateBestMatchedCase::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{  
   BOOST_LOG_TRIVIAL( info ) << "Generating calibrated case: " << m_bmcName << "...";

   // add response
   if ( ErrorHandler::NoError != sa->saveCalibratedCase( m_bmcName.c_str(), m_sampleNum ) )
   { 
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }
   
   BOOST_LOG_TRIVIAL( info ) << "Calibrated case generation " << m_bmcName << " succeeded";
}

void CmdGenerateBestMatchedCase::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <calibratedProjectFileName>\n";
   std::cout << "   - if Monte Carlo simulation was performed, this command will generate the new project file with given name\n";
   std::cout << "     which will have variable parameters value picked up from the best matched case (with minimal RMSE value)\n";
   std::cout << "     of Monte Carlo simulation.\n";
   std::cout << "     Here is an examples of using \"" << cmdName << "\" command:\n";
   std::cout << "         " << cmdName << " \"CalibratedCase.project3d\"\n";
}


