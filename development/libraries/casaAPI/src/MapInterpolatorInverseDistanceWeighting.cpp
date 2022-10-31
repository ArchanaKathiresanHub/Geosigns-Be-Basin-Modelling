//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapInterpolatorInverseDistanceWeighting.h"

#include "ErrorHandler.h"

#include "assert.h"
#include <cmath>
#include <limits>

namespace casa {


MapInterpolatorInverseDistanceWeighting::MapInterpolatorInverseDistanceWeighting(const double idwPower) :
   m_idwPower{idwPower}
{

}

void MapInterpolatorInverseDistanceWeighting::generateInterpolatedMap(const DomainData &domainData,
                                                                      const std::vector<double> &xin,
                                                                      const std::vector<double> &yin,
                                                                      const std::vector<double> &vin,
                                                                      std::vector<double> &xInt,
                                                                      std::vector<double> &yInt,
                                                                      std::vector<double> &vInt) const
{
   if (xin.size() != yin.size() || xin.size() != vin.size())
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "MapInterpolatorInverseDistanceWeighting::generateInterpolatedMap: Input data size mismatch.";
   }

   const double xmin = domainData.xmin;
   const double ymin = domainData.ymin;
   const double deltaX = domainData.deltaX;
   const double deltaY = domainData.deltaY;
   const int    numI = domainData.numI;
   const int    numJ = domainData.numJ;

   const size_t nout = size_t(numI*numJ);
   xInt.resize(nout);
   yInt.resize(nout);
   vInt.resize(nout);

   const double toleranceInMeter = 1.0;
   const size_t nin = vin.size();

   double yval = ymin + 0.5*deltaY;
   size_t k = 0;
   for ( int j = 0; j<numJ; ++j )
   {
      double xval = xmin + 0.5*deltaX;
      for ( int i = 0; i<numI; ++i )
      {
         xInt[k] = xval;
         yInt[k] = yval;

         double v1 = 0.0;
         double s1 = 0.0;
         double closestDist(HUGE_VAL);
         double closestVal(0.0);
         for ( size_t n = 0; n < nin; ++n )
         {
            const double dx = xval - xin[n];
            const double dy = yval - yin[n];
            const double dis = std::sqrt( dx*dx + dy*dy );
                  if (dis < toleranceInMeter)
            {
               v1 = vin[n];
               s1 = 1.0;
               break;
            }
            else
            {
               const double weight = std::pow( 1.0/dis, m_idwPower );
               v1 += vin[n] * weight;
               s1 += weight;
            }
            if (dis < closestDist)
            {
               closestDist = dis;
               closestVal = vin[n];
            }
         }

         if (s1 > std::numeric_limits<double>::epsilon())
         {
            vInt[k] = v1/s1;
         }
         else
         {
            vInt[k] = closestVal;
         }

         ++k;

         xval += deltaX;
      }
      yval += deltaY;
   }
}
}
