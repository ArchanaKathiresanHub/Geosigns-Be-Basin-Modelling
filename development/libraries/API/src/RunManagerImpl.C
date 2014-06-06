//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunManagerImpl.C
/// @brief This file keeps API implementation of Run Manager


#include "RunManagerImpl.h"

namespace casa
{

RunManagerImpl::RunManagerImpl()
{
   ;
}

RunManagerImpl::~RunManagerImpl()
{
   ;
}

// Add Case to set
ErrorHandler::ReturnCode RunManagerImpl::scheduleCase( RunCase & newRun )
{
   return reportError( ErrorHandler::NotImplementedAPI, "scheduleCase() not implemented yet" );
}

// Execute all scheduled cases
ErrorHandler::ReturnCode RunManagerImpl::runScheduledCases( bool asyncRun )
{
   return reportError( ErrorHandler::NotImplementedAPI, "runScheduledCases() not implemented yet" );
}

}
