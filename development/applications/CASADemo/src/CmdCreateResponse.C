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
#include "CmdCreateResponse.h"
#include "CfgFileParser.h"

#include "casaAPI.h"

#include <cstdlib>
#include <iostream>

CmdCreateResponse::CmdCreateResponse( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 4 || m_prms.size() > 5 )
   {
      throw ErrorHandler::Exception( ErrorHandler::RSProxyError ) << "Wrong parameters number: " 
         << m_prms.size() << " (expected 4 or 5) in response proxy " << (m_prms.size() > 0 ? (m_prms[0] + " ") : "" ) << "definition";
   }

   m_targetR2 = 0.95;

   m_proxyName = m_prms[0]; // proxy name
   // convert list of DoEs like: "Tornado,BoxBenken" into array of DoE names
   m_doeList = CfgFileParser::list2array( m_prms[1], ',' );
   m_respSurfOrder = atol( m_prms[2].c_str() ); // response surface order

   if ( m_respSurfOrder < -1 || m_respSurfOrder > 3 )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Supported polynomial order is -1,0,1,2,3, but given is: " << m_respSurfOrder;
   }

   // get kriging name
   if (      !m_prms[3].compare( "No"     ) ) { m_krType = casa::RSProxy::NoKriging;     } // no kriging
   else if ( !m_prms[3].compare( "Local"  ) ) { m_krType = casa::RSProxy::LocalKriging;  } // local kriging
   else if ( !m_prms[3].compare( "Global" ) ) { m_krType = casa::RSProxy::GlobalKriging; } // global kriging
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unsupported kriging type: " << m_prms[3] << ", can be only No, Local or Global";
   }

   if ( m_prms.size() == 5 )
   {
      if ( m_respSurfOrder == -1 ) m_targetR2 = atof( m_prms[4].c_str() );
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Target R2 parameter could be defined only together with automatic search for polynomial order. " <<
           "This is defined by setting the polynomial order to -1. But supplied polynomial order is: " << m_respSurfOrder;
      }
   }
}

void CmdCreateResponse::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{  
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Starting response surface approximation calculation for proxy: " << m_proxyName << std::endl;
      if ( m_respSurfOrder < 0 )
      {
         std::cout << " Automatic search for the polynomial order is set, target R2 value is: " << m_targetR2 << std::endl;
      }
   }

   // add and calculate response
   if ( ErrorHandler::NoError != sa->addRSAlgorithm( m_proxyName.c_str()
                                                   , m_respSurfOrder
                                                   , static_cast<casa::RSProxy::RSKrigingType>( m_krType ) 
                                                   , m_doeList
                                                   , m_targetR2
                                                   )
      )
   { 
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }
   
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Response surface approximation calculation for proxy " << m_proxyName << " finished" << std::endl;
      const casa::RSProxy * proxy = sa->rsProxySet().rsProxy( m_proxyName );
      if ( proxy )
      {
         std::cout << "Polynomial order for response surface approximation is set to: " << proxy->polynomialOrder() << std::endl;
      }
   }
}

