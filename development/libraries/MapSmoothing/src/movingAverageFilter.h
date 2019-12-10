//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "filter.h"

#include <vector>

namespace MapSmoothing
{

class MovingAverageFilter : public Filter
{
public:
  MovingAverageFilter();
  virtual std::vector<std::vector<double>> getMask( const double dx, const double dy, const double smoothingRadius ) const final;
};

} // namespace MapSmoothing
