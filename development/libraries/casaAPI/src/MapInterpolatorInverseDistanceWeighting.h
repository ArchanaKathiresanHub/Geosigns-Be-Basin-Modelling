//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "MapInterpolator.h"

namespace casa
{

class MapInterpolatorInverseDistanceWeighting : public MapInterpolator
{
public:
   MapInterpolatorInverseDistanceWeighting(const double idwPower);
   void generateInterpolatedMap( const DomainData& domainData
                        , const std::vector<double> & xin
                        , const std::vector<double> & yin
                        , const std::vector<double> & vin
                        , std::vector<double>       & xInt
                        , std::vector<double>       & yInt
                        , std::vector<double>       & vInt
                        ) const override;

private:
   double m_idwPower;
};

}
