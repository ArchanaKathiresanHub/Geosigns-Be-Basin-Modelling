// 
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_VALIDATORUNITTESTS_H_
#define _CRUSTALTHICKNESS_VALIDATORUNITTESTS_H_

//Parent
#include "../../DataModel/src/AbstractValidator.h"

/// @class MockValidator class which defines if one node is valid or not according to its member m_isValid
///        Should be used only in unit tests
class MockValidator : public DataModel::AbstractValidator {

   public:

      MockValidator();
      ~MockValidator() = default;

      /// @return m_isValid
      bool isValid( const unsigned int i, const unsigned int j ) const final;

      /// @brief Set the global validity of all node to isValid
      void setIsValid( const bool isValid ) { m_isValid = isValid; };

   private:
      bool m_isValid; ///< Global nodes validity

};
#endif