//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "InterpolationParameters.h"

#include <vector>
#include <string>


namespace mbapi
{
   class Model;
}

namespace casa
{

class MapInterpolator;

class LithologyMapManipulator
{
public:
   LithologyMapManipulator(mbapi::Model& model, const casa::MapInterpolator& interpolator);

   void interpolateLithoMaps(const std::vector<double> &xin,
                             const std::vector<double> &yin, const std::vector<double> &lf1, const std::vector<double> &lf2, const std::vector<double> &lf3,
                             const casa::InterpolationParams& interpolationParams, const std::string &layerName,
                             std::vector<double>& lf1CorrInt,
                             std::vector<double>& lf2CorrInt);


private:
   mbapi::Model& m_model;
   const casa::MapInterpolator& m_interpolator;
};

}
