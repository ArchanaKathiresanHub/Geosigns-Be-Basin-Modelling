//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DATA_MODEL__MOCK_COMPOUNDLITHOLOGYARRAY_H
#define DATA_MODEL__MOCK_COMPOUNDLITHOLOGYARRAY_H

#include <string>
#include <iostream>

#include "AbstractCompoundLithology.h"
#include "MockCompoundLithology.h"

namespace DataModel{

   /// \brief A CompoundLithologyArray.
   class MockCompoundLithologyArray final : public AbstractCompoundLithologyArray{

   public :

      virtual ~MockCompoundLithologyArray() = default;

      /// Mock class returns new MockCompoundLithology
      virtual AbstractCompoundLithology*  operator ()( const unsigned int subscriptI, const unsigned int subscriptJ, const double Age ) const final;


   };

}

inline DataModel::AbstractCompoundLithology*  DataModel::MockCompoundLithologyArray::operator ()( const unsigned int subscriptI, const unsigned int subscriptJ, const double Age ) const{
   return new DataModel::MockCompoundLithology;
}

#endif // DATA_MODEL__MOCK_COMPOUNDLITHOLOGYARRAY_H
