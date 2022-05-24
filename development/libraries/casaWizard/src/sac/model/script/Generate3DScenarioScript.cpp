#include "Generate3DScenarioScript.h"

#include "model/functions/sortedByXWellIndices.h"
#include "model/sacScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

Generate3DScenarioScript::Generate3DScenarioScript(const SACScenario& scenario) :
  CasaScript(scenario.calibrationDirectory()),
  mapsManager_{scenario.mapsManager()},
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
  switch ( mapsManager_.interpolationMethod() )
  {
    case 0:
    {
      command += " IDW " + QString::number(mapsManager_.pIDW());
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

  switch ( mapsManager_.smoothingOption() )
  {
    case 1:
    {
      command += " Gaussian " + QString::number(mapsManager_.radiusSmoothing()) + " 12";
      break;
    }
    case 2:
    {
      command += " MovingAverage " + QString::number(mapsManager_.radiusSmoothing()) + " 12";
      break;
    }
  }
  command += "\n";
  return command;
}

QString Generate3DScenarioScript::setFilterOneDResults() const
{
  QString command("setFilterOneDResults ");

  command += mapsManager_.smartGridding() ? "smartLithoFractionGridding" : "none";

  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();

  int i = 0;
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(ctManager.activeWells());
  for (const Well* well: ctManager.activeWells())
  {
    if (well->isExcluded())
    {
      for (int j = 0; j < sortedIndices.size(); j++)
      {
        if (sortedIndices[j] == i)
        {
          command += QString(" ") + QString::number(j);
          break;
        }
      }
    }
    ++i;
  }
  command += "\n";
  return command;
}

void Generate3DScenarioScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeLoadState(scenario_.runLocation() + "/" + scenario_.iterationDirName() + "/" + scenario_.stateFileNameSAC());
  out << setFilterOneDResults();
  out << writeRunDataDigger();
  out << QString("importOneDResults\n");
  out << generateThreeDFromOneD();
}

bool Generate3DScenarioScript::validateScenario() const
{
  return true;
}

} // namespace sac

} // namespace casaWizard
