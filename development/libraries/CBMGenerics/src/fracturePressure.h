//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CBMGENERICS_FRACTUREPRESSURE_H_
#define _CBMGENERICS_FRACTUREPRESSURE_H_

#include <vector>

using std::vector;

namespace CBMGenerics
{
   namespace fracturePressure
   {

      enum FracturePressureFunctionType
      {
         None,
         FunctionOfDepthWrtSeaLevelSurface,
         FunctionOfDepthWrtSedimentSurface,
         FunctionOfLithostaticPressure
      };

      double hydraulicFracturingFrac( const vector<double> & lithHydraulicFracturingFracs, const vector<double> & fracs );

      double compute( FracturePressureFunctionType type
                    , const vector<double>       & fracturePressureFunctionParameters
                    , const vector<double>       & lithHydraulicFracturingFracs
                    , const vector<double>       & lithFracs
                    , const double               & depthWrtSeaLevel
                    , const double               & depthWrtSedimentSurface
                    , const double               & lithostaticPressure
                    , const double               & hydrostaticPressure
                    , const double& hydrostaticPressureAtSedimentSurface
                    );

      double computeForFunctionOfDepthWrtSeaLevelSurface( const vector<double> & coefficients, const double & depthWrtSeaLevel );
      double computeForFunctionOfDepthWrtSedimentSurface( const vector<double> & coefficients
                                                        , const double         & depthWrtSedimentSurface
                                                        , const double         & hydrostaticPressureAtSedimentSurface
                                                        );

      double computeForFunctionOfLithostaticPressure( const double & hydraulicFracture
                                                    , const double & lithostaticPressure
                                                    , const double & hydrostaticPressure
                                                    );

   } // namespace fracturePressure
} //namespace CBMGenerics

#endif
