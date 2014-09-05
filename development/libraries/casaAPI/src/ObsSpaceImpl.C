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

// Create an observable object which will keep given property value for given XYZ coordinates
Observable * ObsSpace::newObsPropertyXYZ( double x, double y, double z, const char * propName, double simTime )
{
   return new ObsGridPropertyXYZ( x, y, z, propName, simTime );
}

Observable * ObsSpace::newObsPropertyWell( const std::vector<double> & x, const std::vector<double> & y,
                                           const std::vector<double> & z, const char * propName, double simTime )
{
   return new ObsGridPropertyWell( x, y, z, propName, simTime );
}



}
