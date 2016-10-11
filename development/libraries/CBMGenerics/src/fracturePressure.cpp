//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "stdafx.h"

#include "fracturePressure.h"
#include "consts.h"

#include <assert.h>
#include <limits>
#include <cstring>

using std::numeric_limits;

namespace CBMGenerics
{
   namespace fracturePressure
   {

      double hydraulicFracturingFrac( const vector<double> & lithHydraulicFracturingFracs, const vector<double> & lithFracs )
      {
         assert( lithHydraulicFracturingFracs.size() == lithFracs.size() );

         double result = 0.0;
         
         for ( size_t l = 0; l < lithFracs.size(); ++l )
         {
            assert( 0.0 <= lithFracs[l] && lithFracs[l] <= 1.0 );
            result += lithFracs[l] * lithHydraulicFracturingFracs[l];
         }
         return result;
      }

      double compute( FracturePressureFunctionType   type
                    , const vector<double>         & fracturePressureFunctionParameters
                    , const vector<double>         & lithHydraulicFracturingFracs
                    , const vector<double>         & lithFracs
                    , const double                 & depthWrtSeaLevel
                    , const double                 & depthWrtSedimentSurface
                    , const double                 & lithostaticPressure
                    , const double                 & hydrostaticPressure
                    , const double                 & hydrostaticPressureAtSedimentSurface
                    )
      {
         double pressure;
         switch (type)
         {
            case None:
               pressure = numeric_limits<double>::max();
               break;

            case FunctionOfDepthWrtSeaLevelSurface:
               pressure = computeForFunctionOfDepthWrtSeaLevelSurface( fracturePressureFunctionParameters, depthWrtSeaLevel );
               break;

            case FunctionOfDepthWrtSedimentSurface:
               pressure = computeForFunctionOfDepthWrtSedimentSurface( fracturePressureFunctionParameters
                                                                     , depthWrtSedimentSurface
                                                                     , hydrostaticPressureAtSedimentSurface
                                                                     );
               break;

            case FunctionOfLithostaticPressure:
               {
                  double hydraulicFracture = hydraulicFracturingFrac(lithHydraulicFracturingFracs, lithFracs);
                  pressure = computeForFunctionOfLithostaticPressure(hydraulicFracture, lithostaticPressure, hydrostaticPressure);
               }
               break;

            default: assert(0);
         }
         return pressure;
      }

      double computeForFunctionOfDepthWrtSeaLevelSurface( const vector<double> & coefficients, const double & depthWrtSeaLevel )
      {
         assert( coefficients.size() == 4 );
         double pressure = ( ( coefficients[3] * depthWrtSeaLevel + coefficients[2] ) * depthWrtSeaLevel + coefficients[1] ) *
                           depthWrtSeaLevel + coefficients[0];

         // Convert from MPa to Pa:
         return pressure * MPa2Pa;
      }

      double computeForFunctionOfDepthWrtSedimentSurface( const vector<double> & coefficients
                                                        , const double         & depthWrtSedimentSurface
                                                        , const double         & hydrostaticPressureAtSedimentSurface
                                                        )
      {
         assert( coefficients.size() == 4 );
         double pressure = ( ( coefficients[3] * depthWrtSedimentSurface + coefficients[2] ) * depthWrtSedimentSurface + coefficients[1] ) *
                           depthWrtSedimentSurface + coefficients[0];

         // Convert from MPa to Pa:
         return pressure * MPa2Pa + hydrostaticPressureAtSedimentSurface - PressureAtSeaLevel;
      }

      double computeForFunctionOfLithostaticPressure( const double & hydraulicFracture
                                                    , const double & lithostaticPressure
                                                    , const double & hydrostaticPressure
                                                    )
      {
         double pressure = hydraulicFracture * ( lithostaticPressure - hydrostaticPressure ) + hydrostaticPressure;

         // Convert from MPa to Pa:
         return pressure * MPa2Pa;
      }

   } // namespace fracturePressure
} // namespace CBMGenerics
