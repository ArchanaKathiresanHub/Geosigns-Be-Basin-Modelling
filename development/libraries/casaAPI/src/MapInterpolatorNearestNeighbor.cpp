//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapInterpolatorNearestNeighbor.h"

#include "ErrorHandler.h"

#include <cmath>

namespace casa
{

MapInterpolatorNearestNeighbor::MapInterpolatorNearestNeighbor(const int numberOfEdgePointsPerSide, const int nrOfWellsToAverageEdgePoints) :
   m_numberOfEdgePointsPerSide{numberOfEdgePointsPerSide},
   m_nrOfWellsToAverageEdgePoints{nrOfWellsToAverageEdgePoints}
{
}

void MapInterpolatorNearestNeighbor::generateInterpolatedMap(const DomainData &domainData,
                                                    const std::vector<double> &xin,
                                                    const std::vector<double> &yin,
                                                    const std::vector<double> &vin,
                                                    std::vector<double> &xInterpolated,
                                                    std::vector<double> &yInterpolated,
                                                    std::vector<double> &vInterpolated) const
{
   if (xin.size() != yin.size() || xin.size() != vin.size())
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "MapInterpolatorNearestNeighbor::generateInterpolatedMap: Input data size mismatch.";
   }

   std::vector<double> x = xin;
   std::vector<double> y = yin;
   std::vector<double> value = vin;

   const int numI = domainData.numI;
   const int numJ = domainData.numJ;
   double xmin = domainData.xmin;
   double ymin = domainData.ymin;
   double xmax   = xmin + (numI - 1) * domainData.deltaX;
   double ymax   = ymin + (numJ - 1) * domainData.deltaY;

   // Add edge points
   addEdgePoints(xin.size(), xmin, xmax, ymin, ymax, xin, yin, x, y, value);
   std::vector<point> interpolationInputs = generateInterpolationInputs(value, x, y);

   const double wmin = -1.e-3;
   int     numberOfOutputPoints;
   point * outputPoints = nullptr;

   // generate the points, interpolate with NNlib
   points_generate( xmin, xmax, ymin, ymax, numI, numJ, &numberOfOutputPoints, &outputPoints );
   nnpi_interpolate_points( interpolationInputs.size(), interpolationInputs.data(), wmin, numberOfOutputPoints, outputPoints );

   xInterpolated.resize( numberOfOutputPoints );
   yInterpolated.resize( numberOfOutputPoints );
   vInterpolated.resize( numberOfOutputPoints );

   for ( int i = 0; i != numberOfOutputPoints; ++i )
   {
      xInterpolated[i] = outputPoints[i].x;
      yInterpolated[i] = outputPoints[i].y;
      vInterpolated[i] = outputPoints[i].z;
   }

   free( outputPoints );
}

void MapInterpolatorNearestNeighbor::addEdgePoints(const size_t numberOfInputDataPoints, double xmin,
                                                   double xmax,double ymin,  double ymax,
                                                   const std::vector<double> &xin,
                                                   const std::vector<double> &yin,
                                                   std::vector<double>& x, std::vector<double>& y,
                                                   std::vector<double>& value
                                                   ) const
{
   const double deltaXConvexHull = ( xmax - xmin ) / m_numberOfEdgePointsPerSide;
   const double deltaYConvexHull = ( ymax - ymin ) / m_numberOfEdgePointsPerSide;
   for ( int i = 0; i <= m_numberOfEdgePointsPerSide; ++i )
   {
      x.push_back( xmin + deltaXConvexHull * i );
      y.push_back( ymin );
   }
   for ( int i = 1; i < m_numberOfEdgePointsPerSide; ++i )
   {
      x.push_back( xmax );
      y.push_back( ymin + i * deltaYConvexHull );
   }
   for ( int i = m_numberOfEdgePointsPerSide; i >= 0; --i )
   {
      x.push_back( xmin + deltaXConvexHull * i );
      y.push_back( ymax );
   }
   for ( int i = m_numberOfEdgePointsPerSide - 1; i >= 1; --i )
   {
      x.push_back( xmin );
      y.push_back( ymin + i * deltaYConvexHull );
   }

   auto getClosestWells = [=](double x, double y)
   {
      std::vector<int> closestWells;
      std::vector<double> closestDistances;
      for (int i = 0; i<numberOfInputDataPoints; ++i)
      {
         const double dx = x - xin[i];
         const double dy = y - yin[i];
         const double distance2 = dx*dx + dy*dy;

         if (closestWells.size()<m_nrOfWellsToAverageEdgePoints)
         {
            closestWells.push_back(i);
            closestDistances.push_back(distance2);
         }
         else
         {
            int largest = -1;
            double largestDistance = distance2;
            for (int j = 0; j<m_nrOfWellsToAverageEdgePoints; ++j)
            {
               if (closestDistances[j] > largestDistance)
               {
                  largestDistance = closestDistances[j];
                  largest = j;
               }
            }
            if (largest > -1)
            {
               closestWells[largest] = i;
               closestDistances[largest] = distance2;
            }
         }
      }

      return closestWells;
   };

   for ( size_t i = numberOfInputDataPoints; i<x.size(); ++i )
   {
      double vmean = 0.0;
      const std::vector<int> closestWells = getClosestWells(x[i],y[i]);
      for (int j : closestWells)
      {
         vmean += value[j];
      }

      value.push_back( vmean / closestWells.size() );
   }
}

std::vector<point> MapInterpolatorNearestNeighbor::generateInterpolationInputs(std::vector<double> value, std::vector<double> x, std::vector<double> y) const
{
   const double tolerance = 1.e-10;

   std::vector<point> pin;
   size_t j = 0;
   for ( size_t i = 0; i < x.size(); ++i )
   {
      bool noDuplicate = true;
      for ( size_t k = 0; k < j; ++k)
      {
         if (std::fabs(x[i] - pin[k].x) < tolerance && std::fabs(y[i] - pin[k].y) < tolerance)
         {
            pin[k].z = (pin[k].z + value[i]) * 0.5;
            noDuplicate = false;
            break;
         }
      }
      if (noDuplicate)
      {
         point p;
         p.x = x[i];
         p.y = y[i];
         p.z = value[i];
         pin.push_back(p);
         j++;
      }
   }

   return pin;
}

}
