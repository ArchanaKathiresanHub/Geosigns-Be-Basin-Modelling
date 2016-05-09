//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_ABSTRACTVALIDATOR_H_
#define _CRUSTALTHICKNESS_ABSTRACTVALIDATOR_H_

/// @class AbstractValidator Abstract class which defines if one node is valid or not
class AbstractValidator {

   public:
   
      AbstractValidator() {};
      virtual ~AbstractValidator() {};

      virtual const bool isValid( const unsigned int i, const unsigned int j ) const = 0;

};
#endif

