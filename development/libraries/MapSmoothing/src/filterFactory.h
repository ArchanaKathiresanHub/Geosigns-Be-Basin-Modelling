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

namespace MapSmoothing
{

class Filter;

enum class FilterType
{
  Gaussian,
  MovingAverage
};

namespace FilterFactory
{

Filter* createFilter( const FilterType filter );

}

}
