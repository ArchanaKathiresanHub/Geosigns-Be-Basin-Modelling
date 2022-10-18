#include "Generate3DScenarioScript.h"

#include "model/functions/sortedByXWellIndices.h"
#include "model/SacScenario.h"
#include "model/MapsManager.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

Generate3DScenarioScript::Generate3DScenarioScript(const SacScenario& scenario) :
  CasaScript(scenario.calibrationDirectory()),
  m_scenario{scenario}
{
}

const CasaScenario& Generate3DScenarioScript::scenario() const
{
  return m_scenario;
}

QString Generate3DScenarioScript::scriptFilename() const
{
  return QString("generate3DScenarioScript.casa");
}

QString Generate3DScenarioScript::workingDirectory() const
{
  return QString(m_scenario.workingDirectory());
}

QString Generate3DScenarioScript::generateThreeDFromOneD() const
{
  QString command("generateThreeDFromOneD");
  switch ( mapsManager().interpolationMethod() )
  {
    case 0:
    {
      command += " IDW " + QString::number(mapsManager().pIDW());
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

  switch ( mapsManager().smoothingOption() )
  {
    case 1:
    {
      command += " Gaussian " + QString::number(mapsManager().radiusSmoothing()) + " 12";
      break;
    }
    case 2:
    {
      command += " MovingAverage " + QString::number(mapsManager().radiusSmoothing()) + " 12";
      break;
    }
  }
  command += "\n";
  return command;
}

void Generate3DScenarioScript::addWellIndicesToFilter1DResults(QString& command) const
{
   const CalibrationTargetManager& ctManager = m_scenario.calibrationTargetManager();

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
}

void Generate3DScenarioScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeLoadState(m_scenario.runLocation() + "/" + m_scenario.iterationDirName() + "/" + m_scenario.stateFileNameSAC());
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
