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
#include "CmdLoadState.h"

#include "casaAPI.h"
#include "LogHandler.h"

#include <cstdlib>
#include <iostream>

CmdLoadState::CmdLoadState( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No file name for saving CASA state was given";
   }

   m_fileType = "bin";
   m_fileName = m_prms[0];
   if ( m_fileName.empty() ) throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Empty file name for loading CASA state";

   if ( m_prms.size() > 1 )
   {
      m_fileType = m_prms[1];
      if ( !m_fileType.empty() && m_fileType != "bin" && m_fileType != "txt" )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Unknow file type for loading CASA state: " << m_fileType;
      }
   }
}

void CmdLoadState::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Deserializing CASA state from: " << m_fileName << "...";

   sa.reset( casa::ScenarioAnalysis::loadScenario( m_fileName.c_str(), m_fileType.c_str() ) );
   if ( sa->errorCode() != ErrorHandler::NoError )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY )  << "Deserialization succeeded";
}
