//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapSmootherVectorized.h"

namespace MapSmoothing
{

MapSmootherVectorized::MapSmootherVectorized( std::vector<double>& vec, const double undefinedValue,
                                              const double smoothingRadius , const double dx, const double dy,
                                              const unsigned int numI, const unsigned int numJ, const unsigned int nrOfThreads ) :
  MapSmoother(),
  m_vec( vec ),
  m_undefinedValue( undefinedValue ),
  m_smoothingRadius( smoothingRadius ),
  m_dx( dx ),
  m_dy( dy ),
  m_numI( numI ),
  m_numJ( numJ ),
  m_nrOfThreads( nrOfThreads )
{
}

double MapSmootherVectorized::getSmoothingRadius() const
{
  return m_smoothingRadius;
}

double MapSmootherVectorized::getUndefinedValue() const
{
  return m_undefinedValue;
}

unsigned int MapSmootherVectorized::getNrOfThreads() const
{
  return m_nrOfThreads;
}

unsigned int MapSmootherVectorized::getN() const
{
  return m_numI;
}

unsigned int MapSmootherVectorized::getM() const
{
  return m_numI;
}

double MapSmootherVectorized::getDx() const
{
  return m_dx;
}

double MapSmootherVectorized::getDy() const
{
  return m_dy;
}

std::vector<std::vector<double>> MapSmootherVectorized::getMap() const
{
  std::vector<std::vector<double>> grid;
  for ( unsigned int i = 0; i<getN(); ++i )
  {
    std::vector<double> row;
    for ( unsigned int j = 0; j<getM(); ++j )
    {
      row.push_back( m_vec[i * getM() + j] );
    }
    grid.push_back( row );
  }
  return grid;
}

void MapSmootherVectorized::setMapValue( const unsigned int i, const unsigned int j, const double value ) const
{
  m_vec[i * m_numJ + j] = value;
}

} // namespace MapSmoothing
