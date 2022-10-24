//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyMapManipulator.h"

#include "MapInterpolator.h"
#include "cmbAPI.h"

#include <numeric>

namespace casa
{

LithologyMapManipulator::LithologyMapManipulator(mbapi::Model& model, const MapInterpolator &interpolator) :
   m_model{model},
   m_interpolator{interpolator}
{

}


void LithologyMapManipulator::interpolateLithoMaps(const std::vector<double>             & xin,
                                                   const std::vector<double>             & yin,
                                                   const std::vector<double>& lf1,
                                                   const std::vector<double>& lf2,
                                                   const std::vector<double>& lf3,
                                                   const casa::InterpolationParams &interpolationParams,
                                                   const std::string& layerName,
                                                   std::vector<double>& lf1CorrInt,
                                                   std::vector<double>& lf2CorrInt
                                                   )
{
   const size_t nin = xin.size();
   if (nin != yin.size() ||
       nin != lf1.size() ||
       nin != lf2.size() ||
       nin != lf3.size())
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "LithologyMapManipulator::interpolateLithoMaps(): Input data size mismatch.";
   }

   const double shift                = 100.0;

   // for all wells transform lf1, lf2, lf3 to rp, r13
   std::vector<double> rp;
   std::vector<double> r13;
   std::vector<double> x;
   std::vector<double> y;

   // the vertices of the domain must be included as interpolation points
   for ( size_t i = 0; i < nin; ++i )
   {
      const double lf1hat = lf1[i] + shift;
      const double lf2hat = lf2[i] + shift;
      const double lf3hat = lf3[i] + shift;
      x.push_back( xin[i] );
      y.push_back( yin[i] );
      rp.push_back( ( lf1hat + lf3hat ) / lf2hat );
      r13.push_back(  lf1hat / lf3hat );
   }

   // interpolate
   DomainData domainData;
   m_model.highResOrigin(domainData.xmin, domainData.ymin);
   m_model.highResCellSize(domainData.deltaX, domainData.deltaY);
   m_model.highResGridArealSize(domainData.numI, domainData.numJ);

   std::vector<double> rpInt;
   std::vector<double> r13Int;
   std::vector<double> xInt;
   std::vector<double> yInt;

   m_interpolator.generateInterpolatedMap(domainData, xin, yin, rp, xInt, yInt, rpInt);
   m_interpolator.generateInterpolatedMap(domainData, xin, yin, r13, xInt, yInt, r13Int);

   if ( ErrorHandler::NoError != m_model.smoothenVector( rpInt, interpolationParams.smoothingMethod, interpolationParams.radius, interpolationParams.nrOfThreads ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The smoothing of the rp intermediate state failed for the layer : " << layerName;
   }

   if ( ErrorHandler::NoError != m_model.smoothenVector( r13Int, interpolationParams.smoothingMethod, interpolationParams.radius, interpolationParams.nrOfThreads ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The smoothing of the r13 intermediate state failed for the layer : " << layerName;
   }

   // back transform the rpInt and r13Int
   std::vector<double> lf3CorrInt;

   const double sum1 = std::accumulate( lf1.begin(), lf1.end(), 0.0 );
   const double sum2 = std::accumulate( lf2.begin(), lf2.end(), 0.0 );
   const double sum3 = std::accumulate( lf3.begin(), lf3.end(), 0.0 );

   if ( ErrorHandler::NoError != m_model.backTransformLithoFractions( rpInt, r13Int, lf1CorrInt, lf2CorrInt, lf3CorrInt,
                                                                  sum1 < 1.e-5,
                                                                  sum2 < 1.e-5,
                                                                  sum3 < 1.e-5) )
   {
      throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The back transformation of the lithopercentages failed for the layer : "
                                                                  << layerName;
   }
}
}
