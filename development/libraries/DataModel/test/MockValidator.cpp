// 
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "MockValidator.h"

MockValidator::MockValidator()
   :m_isValid( true ) {}

bool MockValidator::isValid( const unsigned int i, const unsigned int j ) const {
   // added to remove gcc warnings
   (void) i;
   (void) j;
   return m_isValid;
}