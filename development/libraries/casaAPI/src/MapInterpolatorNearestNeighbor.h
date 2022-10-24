//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "MapInterpolator.h"
#include "nn.h"

namespace casa
{


class MapInterpolatorNearestNeighbor : public MapInterpolator
{
public:
   MapInterpolatorNearestNeighbor(const int numberOfEdgePointsPerSide = 25, const int nrOfWellsToAverageEdgePoints = 5);

   void generateInterpolatedMap( const DomainData& domainData
                        , const std::vector<double> & xin
                        , const std::vector<double> & yin
                        , const std::vector<double> & vin
                        , std::vector<double>       & xInt
                        , std::vector<double>       & yInt
                        , std::vector<double>       & vInt
                        ) const override;


private:
   void addEdgePoints(const size_t numberOfInputDataPoints,
                      double xmin,
                      double xmax,
                      double ymin,
                      double ymax,
                      const std::vector<double> &xin,
                      const std::vector<double>& yin,
                      std::vector<double>& x,
                      std::vector<double>& y,
                      std::vector<double> &value) const;

   std::vector<point> generateInterpolationInputs(std::vector<double> value, std::vector<double> x, std::vector<double> y) const;

   int m_numberOfEdgePointsPerSide;
   int m_nrOfWellsToAverageEdgePoints;
};

}
