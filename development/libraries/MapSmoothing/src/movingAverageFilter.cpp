//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "movingAverageFilter.h"

namespace MapSmoothing
{

MovingAverageFilter::MovingAverageFilter() :
  Filter()
{
}

std::vector<std::vector<double>> MovingAverageFilter::getMask( const double dx, const double dy, const double smoothingRadius ) const
{
  const double maskSizeX = 2*smoothingRadius/dx + 1;
  const double maskSizeY = 2*smoothingRadius/dy + 1;

  std::vector<std::vector<double>> mask;
  for ( int x = 0; x<maskSizeX; ++x )
  {
    mask.push_back( std::vector<double>( maskSizeY, 1.0 ) );
  }
  return mask;
}

} // namespace MapSmoothing
