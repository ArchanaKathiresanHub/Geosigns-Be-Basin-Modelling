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
#include "CmdAddVarPrm.h"

#include "casaAPI.h"

#include <cstdlib>
#include <iostream>


static casa::VarPrmContinuous::PDF Str2pdf( const std::string & pdf )
{
   if (      pdf == "Block"    ) return casa::VarPrmContinuous::Block;    // equal PDF
   else if ( pdf == "Triangle" ) return casa::VarPrmContinuous::Triangle; // triangle PDF
   else if ( pdf == "Normal"   ) return casa::VarPrmContinuous::Normal;   // gauss PDF

   return casa::VarPrmContinuous::Block;
}



CmdAddVarPrm::CmdAddVarPrm( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No name of variable parameter was given";
   }

   if ( m_prms[0] != "TopCrustHeatProduction" &&
        m_prms[0] != "SourceRockTOC" &&
        m_prms[0] != "CrustThinningOneEvent" )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Unknown variable parameter name: " << m_prms[0];
   }
   // check number of command parameters for var parameter
   if ( m_prms[0] == "TopCrustHeatProduction" && m_prms.size() != 4 ||
        m_prms[0] == "SourceRockTOC"          && m_prms.size() != 5 ||
        m_prms[0] == "CrustThinningOneEvent"  && m_prms.size() != 10
      )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << m_prms[0];
   }
}

void CmdAddVarPrm::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Add variable parameter: " << m_prms[0] << "(";
      for ( size_t i = 1; i < m_prms.size(); ++i )
      {
         std::cout << m_prms[i] << ((i == m_prms.size() - 1) ? "" : ",");
      }
      std::cout << ")" << std::endl;
   }

   casa::VarPrmContinuous::PDF ppdf = casa::VarPrmContinuous::Block;

   if ( m_prms[0] == "TopCrustHeatProduction" )
   {
      double minVal = atof( m_prms[1].c_str() );
      double maxVal = atof( m_prms[2].c_str() );

      ppdf = Str2pdf( m_prms[3] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryTopCrustHeatProduction( *sa.get(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "SourceRockTOC" )
   {
      double minVal = atof( m_prms[2].c_str() );
      double maxVal = atof( m_prms[3].c_str() );

      ppdf = Str2pdf( m_prms[3] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockTOC( *sa.get(), m_prms[1].c_str(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "CrustThinningOneEvent" )
   {
      // Initial crustal thickness
      double minCrustThickn = atof( m_prms[1].c_str() );
      double maxCrustThickn = atof( m_prms[2].c_str() );

      // Start thinning time
      double minTStart = atof( m_prms[3].c_str() );
      double maxTStart = atof( m_prms[4].c_str() );

      // Thinning duration
      double minDeltaT = atof( m_prms[5].c_str() );
      double maxDeltaT = atof( m_prms[6].c_str() );

      double minFactor = atof( m_prms[7].c_str() );
      double maxFactor = atof( m_prms[8].c_str() );

      casa::VarPrmContinuous::PDF pdfType = Str2pdf( m_prms[9] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryOneCrustThinningEvent( *sa.get()
                                                                                          , minCrustThickn
                                                                                          , maxCrustThickn
                                                                                          , minTStart
                                                                                          , maxTStart
                                                                                          , minDeltaT
                                                                                          , maxDeltaT
                                                                                          , minFactor
                                                                                          , maxFactor
                                                                                          , pdfType
                                                                                          )
         ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   }
}
