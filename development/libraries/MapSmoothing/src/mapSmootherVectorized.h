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

#include "mapSmoother.h"

#include <vector>

namespace DataAccess
{
namespace Interface
{
class GridMap;
}
}

namespace MapSmoothing
{

class MapSmootherVectorized : public MapSmoother
{
public:
  MapSmootherVectorized( std::vector<double>& vec, const double undefinedValue, const double smoothingRadius,
                         const double dx, const double dy, const unsigned int numI, const unsigned int numJ,
                         const unsigned int nrOfThreads = 1 );

protected:
  double getDx() const final;
  double getDy() const final;
  double getSmoothingRadius() const final;
  double getUndefinedValue() const final;
  unsigned int getNrOfThreads() const final;
  unsigned int getN() const final;
  unsigned int getM() const final;

  std::vector<std::vector<double>> getMap() const final;
  void setMapValue( unsigned int i, unsigned int j, double value ) const final;

private:
  std::vector<double>& m_vec;
  double m_undefinedValue;
  double m_smoothingRadius;
  double m_dx;
  double m_dy;
  unsigned int m_numI;
  unsigned int m_numJ;
  unsigned int m_nrOfThreads;
};

} // namespace MapSmoothing
