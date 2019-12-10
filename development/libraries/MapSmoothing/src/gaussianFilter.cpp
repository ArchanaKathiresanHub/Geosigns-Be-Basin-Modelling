//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "gaussianFilter.h"

#include <cmath>

namespace MapSmoothing
{

GaussianFilter::GaussianFilter() :
  Filter()
{
}

std::vector<std::vector<double>> GaussianFilter::getMask( const double dx, const double dy, const double smoothingRadius ) const
{
  std::vector<std::vector<double>> mask;

  const double rX = 2.0*smoothingRadius/dx;
  const double rY = 2.0*smoothingRadius/dy;
  const int rXi = static_cast<int>( rX );
  const int rYi = static_cast<int>( rY );
  for ( int x = -rXi; x<=rXi; ++x )
  {
    std::vector<double> gauss;
    for ( int y = -rYi; y<=rYi; ++y )
    {
      gauss.push_back( std::exp( -( x*x*dx*dx + y*y*dy*dy) / ( 2.0*smoothingRadius*smoothingRadius ) ) );
    }
    mask.push_back(gauss);
  }

  return mask;
}

} // namespace MapSmoothing
