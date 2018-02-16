//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DATA_MODEL__ABSTRACT_COMPOUNDLITHOLOGYARRAY_H
#define DATA_MODEL__ABSTRACT_COMPOUNDLITHOLOGYARRAY_H

#include "AbstractCompoundLithology.h"

namespace DataModel {

   class AbstractCompoundLithologyArray {

   public :

      virtual ~AbstractCompoundLithologyArray() = default;
      
      /// @return the compound lithology of the (i,j,age) node
      virtual AbstractCompoundLithology*  operator ()( const unsigned int subscriptI, const unsigned int subscriptJ, const double Age ) const = 0;

   };

} // namespace DataModel

#endif // DATA_MODEL__ABSTRACT_COMPOUNDLITHOLOGYARRAY_H
