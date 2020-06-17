//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

#include "auxiliaryfaulttypes.h"

enum Orientation
{
  frontToFront,
  frontToBack,
  backToFront,
  backToBack
};

namespace DataAccess
{
  namespace Interface
  {
    class FaultCutCreator
    {
    public:
      explicit FaultCutCreator(const std::vector<Point>& intersections);

      ~FaultCutCreator();

      std::vector<PointSequence> createFaultCuts(const double splitDistance);

    private:
      bool mergeFaultCuts(const double splitDistance);
      void sweepLeftRight();
      int findBetterIndexLowerThanCurrent(const unsigned int currentIndex, const PointSequence& orderedFaultCut) const;
      int findBetterIndexHigherThanCurrent(const unsigned int currentIndex, const PointSequence& orderedFaultCut) const;
      void splitCrossing();
      void closeFaultCutLoops(const double splitDistance);

      std::vector<PointSequence> m_faultCuts;
    };
  }
}

