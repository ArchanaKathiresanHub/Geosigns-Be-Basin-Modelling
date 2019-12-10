//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

namespace MapSmoothing
{

class Filter
{
public:
  virtual std::vector<std::vector<double>> getMask( const double dx, const double dy, const double smoothingRadius ) const = 0;
};

} // namespace MapSmoothing
