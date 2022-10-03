//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellTrajectoryExtractor.h"

#include "model/SacScenario.h"

#include <QDir>
#include <QProcess>
#include <QVector>

namespace casaWizard
{

namespace sac
{

WellTrajectoryExtractor::WellTrajectoryExtractor(SacScenario& scenario, const QString& projectName, const QString& iterPath) :
  CaseExtractor(scenario, iterPath),
  m_script{iterationPath(), projectName},
  m_dataCreator{scenario, projectName, iterationPath()}
{
}

WellTrajectoryExtractor::~WellTrajectoryExtractor()
{
}

RunScript& WellTrajectoryExtractor::script()
{
  return m_script;
}

CaseDataCreator& WellTrajectoryExtractor::dataCreator()
{
  return m_dataCreator;
}

void WellTrajectoryExtractor::updateCaseScript(const int wellIndex, const QString relativeDataFolder)
{
  const CalibrationTargetManager& ctManager = scenario().calibrationTargetManager();
  const Well& well = ctManager.well(wellIndex);
  const WellTrajectoryManager& manager = scenario().wellTrajectoryManager();
  const QVector<WellTrajectory> wellTrajectories = manager.trajectoriesType(TrajectoryType::Original1D);
  for (const WellTrajectory& trajectory : wellTrajectories)
  {
    if (trajectory.wellIndex() == well.id())
    {
      m_script.addWell(well.x(), well.y(), ctManager.getCauldronPropertyName(trajectory.propertyUserName()), trajectory.propertyUserName(), relativeDataFolder);
    }
  }
}

} // namespace sac

} // namespace casaWizard
