#include "Generate3DScenarioScript.h"

#include "model/sacScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

Generate3DScenarioScript::Generate3DScenarioScript(const SACScenario& scenario) :
  CasaScript(scenario.calibrationDirectory()),
  scenario_{scenario}
{
}

const CasaScenario& Generate3DScenarioScript::scenario() const
{
  return scenario_;
}

QString Generate3DScenarioScript::scriptFilename() const
{
  return QString("generate3DScenarioScript.casa");
}

QString Generate3DScenarioScript::workingDirectory() const
{
  return QString(scenario_.workingDirectory());
}

QString Generate3DScenarioScript::generateThreeDFromOneD() const
{
  QString command("generateThreeDFromOneD");
  switch ( scenario_.interpolationMethod() )
  {
    case 0:
    {
      command += " IDW " + QString::number(scenario_.pIDW());
      break;
    }
    case 1:
    {
      command += " NN";
      break;
    }
    default:
    {
      return "";
    }
  }

  switch ( scenario_.smoothingOption() )
  {
    case 1:
    {
      command += " Gaussian " + QString::number(scenario_.radiusSmoothing()) + " " + QString::number(scenario_.threadsSmoothing());
      break;
    }
    case 2:
    {
      command += " MovingAverage " + QString::number(scenario_.radiusSmoothing()) + " " + QString::number(scenario_.threadsSmoothing());
      break;
    }
  }
  command += "\n";
  return command;
}

QString Generate3DScenarioScript::setFilterOneDResults() const
{
  QString command("setFilterOneDResults smartLithoFractionGridding");

  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  const QVector<const Well*>& wells = ctManager.wells();
  const WellTrajectoryManager& mgr = scenario_.wellTrajectoryManager();
  int i = 0;
  for (const WellTrajectory& wellTrajectory: mgr.trajectoriesType(TrajectoryType::Original1D))
  {
    const Well* well = wells[wellTrajectory.wellIndex()];
    if ( well->isActive() )
    {
      if (well->isExcluded())
      {
        command += QString(" ") + QString::number(i);
      }
      ++i;
    }
  }
  command += "\n";
  return command;
}

void Generate3DScenarioScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeLoadState(scenario_.runLocation() + "/" + scenario_.iterationDirName() + "/" + scenario_.stateFileNameSAC());
  out << writeRunDataDigger();
  out << QString("importOneDResults\n");
  out << setFilterOneDResults();
  out << generateThreeDFromOneD();
}

bool Generate3DScenarioScript::validateScenario() const
{
  return true;
}

} // namespace sac

} // namespace casaWizard
