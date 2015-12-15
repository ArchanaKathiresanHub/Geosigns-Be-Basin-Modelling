// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_EXPDESIGN_H
#define SUMLIB_EXPDESIGN_H

#include <vector>
#include <string>

#include "SUMlib.h"

namespace SUMlib {

class Case;
class ParameterBounds;

/// @class ExpDesign is an algorithm that generates a set of cases
/// based on the parameter bounds
class INTERFACE_SUMLIB ExpDesign
{
   public:

      virtual ~ExpDesign()
      {}

      virtual void getCaseSet( ParameterBounds const& bounds, Case const& center,
                               bool replicate, std::vector<Case>& caseSet ) const = 0;
      virtual unsigned int getNbOfCases( ParameterBounds const& bounds,
                                         bool replicate = true ) const = 0;
      virtual std::string toString() const = 0;
};

} // namespace SUMlib

#endif // SUMLIB_EXPDESIGN_H
