//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Validator.h"

Validator::Validator( GeoPhysics::ProjectHandle& projectHandle ) :
   m_projectHandle( projectHandle ){};

bool Validator::isValid( const unsigned int i, const unsigned int j ) const{
   return m_projectHandle.getNodeIsValid( i, j );
}