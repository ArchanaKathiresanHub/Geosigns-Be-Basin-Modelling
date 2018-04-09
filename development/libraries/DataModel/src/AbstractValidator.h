//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DATAMODEL_ABSTRACTVALIDATOR_H
#define DATAMODEL_ABSTRACTVALIDATOR_H

/// @class AbstractValidator Abstract class which defines if one node is valid or not
namespace DataModel {
   class AbstractValidator {

      public:

         AbstractValidator() = default;
         virtual ~AbstractValidator() = default;

         virtual bool isValid( const unsigned int i, const unsigned int j ) const = 0;

   };
}

#endif

