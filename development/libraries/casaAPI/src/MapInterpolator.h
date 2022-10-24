//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

namespace casa
{

struct DomainData
{
   DomainData():
      xmin(0),
      ymin(0),
      deltaX(0),
      deltaY(0),
      numI(0),
      numJ(0)
   {}

   double xmin;
   double ymin;
   double deltaX;
   double deltaY;
   long    numI;
   long    numJ;
};

class MapInterpolator
{
public:
   MapInterpolator() = default;

   virtual void generateInterpolatedMap(  const DomainData& domainData
                                 , const std::vector<double> & xin
                                 , const std::vector<double> & yin
                                 , const std::vector<double> & vin
                                 , std::vector<double>       & xInt
                                 , std::vector<double>       & yInt
                                 , std::vector<double>       & vInt
                                 ) const = 0;
};

} // namespace casa
