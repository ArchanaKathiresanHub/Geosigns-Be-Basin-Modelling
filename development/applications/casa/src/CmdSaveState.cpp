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
#include "CmdSaveState.h"

#include "casaAPI.h"
#include "LogHandler.h"

#include <cstdlib>
#include <iostream>

CmdSaveState::CmdSaveState( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_fileType = "bin";
   m_fileName = m_prms.size() > 0 ? m_prms[0] : "";

   if ( m_fileName.empty() ) throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Empty file name to save CASA state";

   if ( m_prms.size() > 1 )
   {
      m_fileType = m_prms[1];
      if ( m_fileType.empty() || (m_fileType != "bin" && m_fileType != "txt" ) )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Unknow file type to save CASA state: " << m_fileType;
      }
   }
}

void CmdSaveState::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) <<  "Serializing CASA state to: " << m_fileName << "...";
   
   if ( ErrorHandler::NoError != sa->saveScenario( m_fileName.c_str(), m_fileType.c_str() ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << "Serializing succeeded";
}
