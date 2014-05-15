//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RSProxyImpl.C
/// @brief This file keeps API implementation of Response Surface Proxy


#include "RSProxyImpl.h"

namespace casa
{

RSProxyImpl::RSProxyImpl()
{
   ;
}

RSProxyImpl::~RSProxyImpl()
{
   ;
}

// Calculate polynomial coefficients for the given cases set
ErrorHandler::ReturnCode RSProxyImpl::calculateRSProxy( const std::vector<RunCase*> & caseSet )
{
   return ReportError( ErrorHandler::NotImplementedAPI, "calculateRSProxy() not implemented yet" );
}

// Calculate values of observables for given set of parameters
ErrorHandler::ReturnCode RSProxyImpl::evaluateRSProxy( RunCase & cs )
{
   return ReportError( ErrorHandler::NotImplementedAPI, "evaluateRSProxy() not implemented yet" );
}

}
