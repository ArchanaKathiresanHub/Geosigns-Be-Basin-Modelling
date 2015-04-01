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

#include <cstdlib>
#include <iostream>

CmdDoE::CmdDoE( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No name of DoE was given";
   }

   m_numExp = 0;

   if (      m_prms[0] == "Tornado"              ) { m_doeAlg = casa::DoEGenerator::Tornado; }
   else if ( m_prms[0] == "BoxBehnken"           ) { m_doeAlg = casa::DoEGenerator::BoxBehnken; }
   else if ( m_prms[0] == "PlackettBurman"       ) { m_doeAlg = casa::DoEGenerator::PlackettBurman; }
   else if ( m_prms[0] == "PlackettBurmanMirror" ) { m_doeAlg = casa::DoEGenerator::PlackettBurmanMirror; }
   else if ( m_prms[0] == "FullFactorial"        ) { m_doeAlg = casa::DoEGenerator::FullFactorial; }
   else if ( m_prms[0] == "LatinHypercube"       )
   {
      m_doeAlg = casa::DoEGenerator::LatinHypercube;
      if ( m_prms.size() > 1 ) m_numExp = atol( m_prms[1].c_str() );
   }
   else if ( m_prms[0] == "SpaceFilling" )
   {
      m_doeAlg = casa::DoEGenerator::SpaceFilling;
      if ( m_prms.size() > 1 ) m_numExp = atol( m_prms[1].c_str() );
   }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown cauldron DoE name: " << m_prms[0];
   }
}

void CmdDoE::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa ) 
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Generating " << m_prms[0] << " DoE ... ";
   }

   if ( ErrorHandler::NoError != sa->setDoEAlgorithm( static_cast<casa::DoEGenerator::DoEAlgorithm>( m_doeAlg ) ) ||
        ErrorHandler::NoError != sa->doeGenerator().generateDoE( sa->varSpace(), sa->doeCaseSet(), m_numExp ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      sa->doeCaseSet().filterByExperimentName( casa::DoEGenerator::DoEName( static_cast<casa::DoEGenerator::DoEAlgorithm>( m_doeAlg ) ) );
      std::cout << "\n  Generated " << sa->doeCaseSet().size() << " cases" << std::endl;
      sa->doeCaseSet().filterByExperimentName( "" );
   }
}

