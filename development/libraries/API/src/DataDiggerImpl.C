//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataDiggerImpl.h
/// @brief This file keeps API implementation for data digger. 


#include "DataDiggerImpl.h"

namespace casa
{

DataDiggerImpl::DataDiggerImpl()
{
   ;
}

DataDiggerImpl::~DataDiggerImpl()
{
   ;
}

// Add Case to set
ErrorHandler::ReturnCode DataDiggerImpl::collectRunResults( RunCase & cs )
{
   return ReportError( ErrorHandler::NotImplementedAPI, "collectRunResults() not implemented yet" );
}

}
