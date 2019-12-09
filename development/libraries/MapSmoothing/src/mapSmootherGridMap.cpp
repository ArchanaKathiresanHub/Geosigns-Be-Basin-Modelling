//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapSmootherGridMap.h"

#include "GridMap.h"

namespace MapSmoothing
{

MapSmootherGridMap::MapSmootherGridMap( DataAccess::Interface::GridMap* gridMap, const double smoothingRadius, const unsigned int nrOfThreads ) :
  MapSmoother(),
  m_gridMap( gridMap ),
  m_smoothingRadius( smoothingRadius ),
  m_nrOfThreads( nrOfThreads )
{
}

double MapSmootherGridMap::getSmoothingRadius() const
{
  return m_smoothingRadius;
}

double MapSmootherGridMap::getUndefinedValue() const
{
  return m_gridMap->getUndefinedValue();
}

unsigned int MapSmootherGridMap::getNrOfThreads() const
{
  return m_nrOfThreads;
}

unsigned int MapSmootherGridMap::getN() const
{
  return m_gridMap->numI();
}

unsigned int MapSmootherGridMap::getM() const
{
  return m_gridMap->numJ();
}

double MapSmootherGridMap::getDx() const
{
  return m_gridMap->deltaI();
}

double MapSmootherGridMap::getDy() const
{
  return m_gridMap->deltaJ();
}

std::vector<std::vector<double>> MapSmootherGridMap::getMap() const
{
  std::vector<std::vector<double>> grid;
  for ( unsigned int i = 0; i<getN(); ++i )
  {
    std::vector<double> row;
    for ( unsigned int j = 0; j<getM(); ++j )
    {
      row.push_back( m_gridMap->getValue( i, j ) );
    }
    grid.push_back( row );
  }
  return grid;
}

void MapSmootherGridMap::setMapValue( const unsigned int i, const unsigned int j, const double value ) const
{
  m_gridMap->setValue( i, j, value );
}

} // namespace MapSmoothing
