#include "optimalCaseScript.h"

#include "model/logger.h"
#include "model/uaScenario.h"

#include <QTextStream>
#include <QFile>
#include <QFileInfo>

namespace casaWizard
{

namespace ua
{

OptimalCaseScript::OptimalCaseScript(const UAScenario& scenario) :
  CasaScript(scenario.workingDirectory()),
  scenario_{scenario},
  optimalCaseDirectory_{scenario_.workingDirectory() + "/" + scenario_.runLocation() + "/Calibrated_calibrated"}
{
}

const CasaScenario& OptimalCaseScript::scenario() const
{
  return scenario_;
}

QString OptimalCaseScript::scriptFilename() const
{
  return QString("optimalCaseScript.casa");
}

QString OptimalCaseScript::workingDirectory() const
{
  return QString(scenario_.workingDirectory());
}

QString OptimalCaseScript::optimalCaseDirectory() const
{
  return optimalCaseDirectory_;
}

bool OptimalCaseScript::generateCommands()
{
  addCommand("rm -rf " + optimalCaseDirectory_);
  return CasaScript::generateCommands();
}

bool OptimalCaseScript::validateScenario() const
{
  QString stateFilename{scenario_.stateFileNameMCMC()};
  QFileInfo checkFile(scenario_.workingDirectory() + "/" + stateFilename);
  if (!checkFile.exists())
  {
    Logger::log() << "State file \"" << stateFilename << "\" for MCMC run not found. Please run the calibration first" << Logger::endl();
    return false;
  }

  if (!checkFile.isFile())
  {
    Logger::log() << "Path to state file is not a file" << Logger::endl();
    return false;
  }
  return true;
}

void OptimalCaseScript::writeScriptContents(QFile& file) const
{
  QString filename{"calibrated.project3d"};
  QTextStream out(&file);
  out << writeBaseProject(scenario_.project3dPath());
  out << writeLoadState(scenario_.stateFileNameMCMC());
  out << "generateCalibratedCase \"" + filename + "\" 1\n";

  Logger::log() << "- \""  << filename << "\" will be in \"" << scenario_.runLocation() << "\"" << Logger::endl();
}

} // namespace ua

} // namespace casaWizard
