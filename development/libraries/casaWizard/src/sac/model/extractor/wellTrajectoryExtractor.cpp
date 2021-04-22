#include "wellTrajectoryExtractor.h"

#include "model/sacScenario.h"
#include "model/script/track1dScript.h"
#include "model/input/wellTrajectoryDataCreator.h"

#include <QDir>
#include <QProcess>

namespace casaWizard
{

namespace sac
{

WellTrajectoryExtractor::WellTrajectoryExtractor(SACScenario& scenario, const QString& projectName, const QString& iterPath) :
  CaseExtractor(scenario, iterPath),
  script_{iterationPath(), projectName},
  dataCreator_{scenario, projectName, iterationPath()}
{
}

WellTrajectoryExtractor::~WellTrajectoryExtractor()
{
}

RunScript& WellTrajectoryExtractor::script()
{
  return script_;
}

CaseDataCreator& WellTrajectoryExtractor::dataCreator()
{
  return dataCreator_;
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
      script_.addWell(well.x(), well.y(), trajectory.property(), relativeDataFolder);
    }
  }
}

} // namespace sac

} // namespace casaWizard
