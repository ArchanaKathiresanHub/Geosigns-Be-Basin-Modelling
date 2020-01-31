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

#include "filterFactory.h"

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

class MapSmoother
{
public:
  void doSmoothing( const FilterType& filterType ) const;

protected:
  virtual double getDx() const = 0;
  virtual double getDy() const = 0;
  virtual double getSmoothingRadius() const = 0;
  virtual double getUndefinedValue() const = 0;
  virtual unsigned int getNrOfThreads() const = 0;
  virtual unsigned int getN() const = 0;
  virtual unsigned int getM() const = 0;
  virtual std::vector<std::vector<double>> getMap() const = 0; // Vector of column vectors
  virtual void setMapValue( unsigned int i, unsigned int j, double value ) const = 0;
};

}
