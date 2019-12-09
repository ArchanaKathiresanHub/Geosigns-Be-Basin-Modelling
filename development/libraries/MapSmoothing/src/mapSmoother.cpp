//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapSmoother.h"

#include "filter.h"

#include "GridMap.h"

#include <thread>

#include <cmath>
#include <vector>

namespace MapSmoothing
{

void MapSmoother::doSmoothing( const FilterType& filterType ) const
{
  std::unique_ptr<Filter> filter( FilterFactory::createFilter( filterType ) );
  if ( !filter || getSmoothingRadius() == 0.0 )
  {
    return;
  }

  const std::vector<std::vector<double>> grid = getMap();

  const double undefinedValue = getUndefinedValue();
  const unsigned int n = getN();
  const unsigned int m = getM();

  const std::vector<std::vector<double>> mask = filter->getMask( getDx(), getDy(), getSmoothingRadius() );
  const unsigned int maskSizeX = mask.size();
  const unsigned int maskSizeY = mask[0].size();
  const unsigned int rX = (maskSizeX - 1)/2;
  const unsigned int rY = (maskSizeY - 1)/2;

  auto smoothen = [&, this]( int threadId )
  {
    for ( unsigned int i = threadId; i<n; i+= getNrOfThreads() )
    {
      for ( unsigned int j = 0; j<m; ++j )
      {
        if ( grid[i][j] != undefinedValue )
        {
          double val = 0.0;
          double div = 0.0;

          for ( unsigned int k = 0; k<maskSizeX; ++k )
          {
            const unsigned int ii = std::max( rX, std::min( n - 1 + rX, k + i ) );

            const std::vector<double>& gridRow = grid[ ii - rX ];
            const std::vector<double>& maskRow = mask[ ii - i ];

            for ( unsigned int l = 0; l<maskSizeY; ++l )
            {
              const unsigned int jj = std::max( rY, std::min( m - 1 + rY, l + j ) );

              const double value = gridRow[ jj - rY ];
              if ( value == undefinedValue )
              {
                // Ignore neighbors with undefined value
                continue;
              }

              const double f = maskRow[jj - j];
              val += value * f;
              div += f;
            }
          }
          setMapValue( i, j, val/div );
        }
      }
    }
  };

  std::vector<std::thread*> threads;
  for ( unsigned int threadId = 1; threadId < getNrOfThreads(); ++threadId )
  {
    threads.push_back( new std::thread( [&, this, threadId](){ smoothen( threadId ); } ) );
  }
  smoothen( 0 ); // Use master thread also, resulting in no threading for nr of threads = 1

  for ( std::thread* t : threads )
  {
    t->join();
    delete t;
  }
}

} // namespace MapSmoothing
