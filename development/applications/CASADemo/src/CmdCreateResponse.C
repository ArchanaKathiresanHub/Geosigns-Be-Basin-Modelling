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
   assert( m_prms.size() == 4 );

   m_proxyName = m_prms[0]; // proxy name
   // convert list of DoEs like: "Tornado,BoxBenken" into array of DoE names
   m_doeList = CfgFileParser::list2array( m_prms[1], ',' );
   m_respSurfOrder = atol( m_prms[2].c_str() ); // response surface order

   // get kriging name
   if (      !m_prms[3].compare( "No"     ) ) { m_krType = casa::RSProxy::NoKriging;     } // no kriging
   else if ( !m_prms[3].compare( "Local"  ) ) { m_krType = casa::RSProxy::LocalKriging;  } // local kriging
   else if ( !m_prms[3].compare( "Global" ) ) { m_krType = casa::RSProxy::GlobalKriging; } // global kriging
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unsupported kriging type: " << m_prms[3] << ", can be only No, Local or Global";
   }
}

void CmdCreateResponse::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{  
   // add response
   if ( ErrorHandler::NoError != sa->addRSAlgorithm( m_proxyName.c_str()
                                                   , m_respSurfOrder
                                                   , static_cast<casa::RSProxy::RSKrigingType>( m_krType ) 
                                                   )
      ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   
   // call response calculation
   casa::RSProxy * proxy = sa->rsProxySet().rsProxy( m_proxyName.c_str() );

   std::vector<const casa::RunCase *> rcs; // set of run cases which will be used in RSProxy calculation

   // collect cases for given set of DoE
   for ( size_t i = 0; i < m_doeList.size(); ++i )
   {
      sa->doeCaseSet().filterByExperimentName( m_doeList[i] );
      for ( size_t j = 0; j < sa->doeCaseSet().size(); ++j )
      {
         rcs.push_back( sa->doeCaseSet()[j] );
      }
   }
   if ( rcs.empty() && m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Empty cases set for the given set of DoEs, skip proxy calculation" << std::endl;
   }
   else
   {
      if ( m_commander.verboseLevel() > CasaCommander::Quiet )
      {
         std::cout << "Starting response surface approximation calculation for proxy: " << m_proxyName << std::endl;
      }

      // Here we will calculate proxy, at least!
      if ( ErrorHandler::NoError != proxy->calculateRSProxy( rcs ) )
      {
         throw ErrorHandler::Exception( proxy->errorCode() ) << proxy->errorMessage();
      }

      if ( m_commander.verboseLevel() > CasaCommander::Quiet )
      {
         std::cout << "Response surface approximation calculation for proxy " << m_proxyName << " finished" << std::endl;
      }
   }
}
