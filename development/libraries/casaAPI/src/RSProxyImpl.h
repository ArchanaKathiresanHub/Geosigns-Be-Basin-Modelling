//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RSProxyImpl.h
/// @brief This file keeps API implementation declarations for response surface proxy

#ifndef CASA_API_RESPONSE_SURFACE_PROXY_IMPL_H
#define CASA_API_RESPONSE_SURFACE_PROXY_IMPL_H

#include "RSProxy.h"

namespace casa
{   
   // Class to implement handling of response surface proxy
   class RSProxyImpl : public RSProxy
   {
   public:

      // Constructor / Destructor
      RSProxyImpl();
      virtual ~RSProxyImpl();

      // Calculate polynomial coefficients for the given cases set
      // caseSet list of cases which keeps simulation results with variable parameters value and observables value
      // return ErrorHandler::NoError in case of success, or error code in case of error
      virtual ErrorHandler::ReturnCode calculateRSProxy( const std::vector<RunCase*> & caseSet );

      // Calculate values of observables for given set of parameters
      // cs case which keeps list of parameters and list of observables to be calculated
      // return ErrorHandler::NoError in case of success, or error code in case of error
      virtual ErrorHandler::ReturnCode evaluateRSProxy( RunCase & cs );

   protected:

   private:
   };
}

#endif // CASA_API_RESPONSE_SURFACE_PROXY_IMPL_H
