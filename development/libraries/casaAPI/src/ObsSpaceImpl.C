//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsSpaceImpl.C
/// @brief This file keeps API implementation for observables manager. 

#include "ObsSpaceImpl.h"

#include "ObsGridPropertyXYZ.h"
#include "ObsGridPropertyWell.h"

namespace casa
{

ErrorHandler::ReturnCode ObsSpaceImpl::addObservable( Observable * prm )
{
   if ( prm ) { m_obsSet.push_back( prm ); }
   else       { return reportError( UndefinedValue, "ObsSpaceImpl::addObservable() no observable was given" ); }
   return NoError;
}

}
