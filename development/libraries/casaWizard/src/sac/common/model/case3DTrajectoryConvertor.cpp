//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "case3DTrajectoryConvertor.h"

#include "input/case3DTrajectoryReader.h"
#include "SacScenario.h"

#include <assert.h>
#include <cmath>

namespace casaWizard
{

namespace sac
{

namespace case3DTrajectoryConvertor
{

void convertToScenario(const Case3DTrajectoryReader& reader, SacScenario& scenario, bool optimized)
{
  const double epsilon{1}; // Track 1d accuracy

  WellTrajectoryManager& manager = scenario.wellTrajectoryManager();
  const QVector<const Well*> wells = scenario.calibrationTargetManager().wells();
  QVector<double> x = reader.x();
  QVector<double> y = reader.y();
  QVector<double> depth = reader.depth();

  const int size = x.size();
  assert(size == y.size());
  assert(size == depth.size());

  const QVector<WellTrajectory> trajectories = manager.trajectories()[optimized? TrajectoryType::Optimized3D: TrajectoryType::Original3D];
  double currentX{0};
  double currentY{0};

  for (const WellTrajectory& traj : trajectories)
  {
    const int wellId = traj.wellIndex();
    currentX = wells[wellId]->x();
    currentY = wells[wellId]->y();

    int iStart = 0;
    int iEnd = 0;
    bool foundStart = false;
    bool foundEnd = false;

    // Find first and last index of the coordinates belonging to this well
    for( int i=0; i<size && !foundEnd; ++i)
    {
      if (!foundStart)
      {
        foundStart = std::fabs(currentX-x[i])<epsilon &&
                     std::fabs(currentY-y[i])<epsilon;
        iStart = i;
      }
      else
      {
        foundEnd = std::fabs(currentX-x[i])>epsilon ||
                   std::fabs(currentY-y[i])>epsilon;
        iEnd = i-1;
      }
    }
    if (!foundEnd)
    {
      iEnd = size-1;
    }

    const QVector<double> propValue = reader.readProperty(scenario.calibrationTargetManager().getCauldronPropertyName(traj.propertyUserName()));

    if (propValue.size() <= iEnd)
    {
      throw std::range_error("case3DTrajectoryConverter: Property value size does not match with trajectory size.");
    }

    QVector<double> d(iEnd - iStart + 1, 0);
    QVector<double> v(iEnd - iStart + 1, 0);
    for (int j = iStart; j <= iEnd; ++j)
    {
      d[j - iStart] = depth[j];
      v[j - iStart] = propValue[j];
    }
    if (optimized)
    {
      manager.setTrajectoryData(TrajectoryType::Optimized3D, traj.trajectoryIndex(), d, v);
    }
    else
    {
      manager.setTrajectoryData(TrajectoryType::Original3D, traj.trajectoryIndex(), d, v);
    }
  }
}

}

} // namespace sac

} // namespace casaWizard
