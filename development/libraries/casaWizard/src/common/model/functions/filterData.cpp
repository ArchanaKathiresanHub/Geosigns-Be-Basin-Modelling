//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "filterData.h"

#include "cmath"

namespace casaWizard
{

namespace functions
{

QVector<double> smoothenData(const QVector<double>& depths, const QVector<double>& values, const double radius)
{
  QVector<double> smoothenedData;

  if (radius < 1.0) return smoothenedData;

  for ( const double depth : depths )
  {
    double value = 0.0;
    double gaussTotal = 0.0;
    for ( int i = 0; i < depths.size(); ++i )
    {
      const double dx = depth - depths[i];
      if (dx > radius * 4.0) continue;
      const double gauss = std::exp( -(dx*dx) / ( 2.0*radius*radius ) );
      value += values[i] * gauss;
      gaussTotal += gauss;
    }
    if (gaussTotal > 0.0)
    {
      smoothenedData.push_back( value / gaussTotal );
    }
  }

  return smoothenedData;
}

QVector<int> subsampleData(const QVector<double>& depths, const double length)
{
  //Returns the indices of the remaining data points
  QVector<int> subsampleData;

  if (depths.empty()) return subsampleData;

  double prevDepth = -1.0e10;
  for (int i=0; i<depths.size(); ++i)
  {
    if (depths[i]>prevDepth+length)
    {
      subsampleData.push_back(i);
      prevDepth=depths[i];
    }
  }

  return subsampleData;
}

}  // namespace functions

}  // namespace casaWizard
