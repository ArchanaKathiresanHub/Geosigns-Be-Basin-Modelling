//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS_VALIDATOR_H
#define GEOPHYSICS_VALIDATOR_H

//Geophysics
#include "GeoPhysicsProjectHandle.h"

//Parent
#include "AbstractValidator.h"

/// @class Validator class which defines if one node is valid or not according to Geophysics project handle
class Validator : public AbstractValidator  {

   public:
   
      Validator( GeoPhysics::ProjectHandle& projectHandle );
      ~Validator() {};

      /// @brief return true if (i,j) is a valid node, else return false
      bool isValid( const unsigned int i, const unsigned int j ) const;

   private:
      GeoPhysics::ProjectHandle& m_projectHandle;

};
#endif

