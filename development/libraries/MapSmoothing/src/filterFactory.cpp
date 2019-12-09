//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "filterFactory.h"

#include "gaussianFilter.h"
#include "movingAverageFilter.h"

namespace MapSmoothing
{

namespace FilterFactory
{

Filter* createFilter(const FilterType filter )
{
  switch ( filter )
  {
    case FilterType::Gaussian:
    {
      return new GaussianFilter();
    }
    case FilterType::MovingAverage:
    {
      return new MovingAverageFilter();
    }
    default:
    {
      return nullptr;
    }
  }
}

} // namespace FilterFactory

} // namespace MapSmoothing
