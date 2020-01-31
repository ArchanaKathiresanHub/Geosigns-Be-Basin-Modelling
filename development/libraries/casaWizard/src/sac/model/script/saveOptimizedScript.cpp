#include "saveOptimizedScript.h"

#include "model/sacScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

SaveOptimizedScript::SaveOptimizedScript(const SACScenario& scenario) :
  CasaScript(scenario.calibrationDirectory()),
  scenario_{scenario}
{
}

const CasaScenario& SaveOptimizedScript::scenario() const
{
  return scenario_;
}

QString SaveOptimizedScript::scriptFilename() const
{
  return QString("saveOptimizedScript.casa");
}

QString SaveOptimizedScript::workingDirectory() const
{
  return QString(scenario_.workingDirectory());
}

void SaveOptimizedScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeLoadState(scenario_.runLocation() + "/" + scenario_.iterationDirName() + "/" + scenario_.stateFileNameSAC());
  out << writeRunDataDigger();
  out << QString("importOneDResults\n");
  out << QString("setFilterOneDResults smartLithoFractionGridding\n");
  out << QString("generateThreeDFromOneD\n");
}

bool SaveOptimizedScript::validateScenario() const
{
  return true;
}

} // namespace sac

} // namespace casaWizard
