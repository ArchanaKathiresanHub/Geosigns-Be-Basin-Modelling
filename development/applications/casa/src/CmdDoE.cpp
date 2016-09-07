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
#include "CmdDoE.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include <cstdlib>
#include <iostream>

CmdDoE::CmdDoE( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No name of DoE was given";
   }

   m_numExp = 0;

   if (      m_prms[0].find( "Tornado"             , 0 ) != std::string::npos ) { m_doeAlg = casa::DoEGenerator::Tornado; }
   else if ( m_prms[0].find( "BoxBehnken"          , 0 ) != std::string::npos ) { m_doeAlg = casa::DoEGenerator::BoxBehnken; }
   else if ( m_prms[0].find( "PlackettBurman"      , 0 ) != std::string::npos ) { m_doeAlg = casa::DoEGenerator::PlackettBurman; }
   else if ( m_prms[0].find( "PlackettBurmanMirror", 0 ) != std::string::npos ) { m_doeAlg = casa::DoEGenerator::PlackettBurmanMirror; }
   else if ( m_prms[0].find( "FullFactorial"       , 0 ) != std::string::npos ) { m_doeAlg = casa::DoEGenerator::FullFactorial; }
   else if ( m_prms[0].find( "LatinHypercube"      , 0 ) != std::string::npos )
   {
      m_doeAlg = casa::DoEGenerator::LatinHypercube;
      if ( m_prms.size() > 1 ) m_numExp = atol( m_prms[1].c_str() );
   }
   else if ( m_prms[0].find( "SpaceFilling", 0 ) != std::string::npos )
   {
      m_doeAlg = casa::DoEGenerator::SpaceFilling;
      if ( m_prms.size() > 1 ) m_numExp = atol( m_prms[1].c_str() );
   }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown cauldron DoE name: " << m_prms[0];
   }
}

void CmdDoE::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa ) 
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Generating parameters sets for " << m_prms[0] << " DoE ... ";

   if ( ErrorHandler::NoError != sa->setDoEAlgorithm( static_cast<casa::DoEGenerator::DoEAlgorithm>( m_doeAlg ) ) ||
        ErrorHandler::NoError != sa->doeGenerator().generateDoE( sa->varSpace(), sa->doeCaseSet(), m_numExp, m_prms[0] ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }

   LogHandler( LogHandler::INFO_SEVERITY ) << m_prms[0] << " DoE generation was finished";

   sa->doeCaseSet().filterByExperimentName( casa::DoEGenerator::DoEName( static_cast<casa::DoEGenerator::DoEAlgorithm>( m_doeAlg ) ) );
   LogHandler( LogHandler::DEBUG_SEVERITY ) << sa->doeCaseSet().size() << " parameters sets were genereated";
   sa->doeCaseSet().filterByExperimentName( "" );
}

