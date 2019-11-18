#include "optimalCaseScript.h"

#include "model/logger.h"
#include "model/uaScenario.h"

#include <QDir>
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
  const QString runFolderStr = scenario_.workingDirectory() + "/" + scenario_.runLocation();
  const QDir runFolderDir{runFolderStr};

  if (!runFolderDir.exists() ||
      !runFolderDir.isReadable())
  {
    return false;
  }

  const QFileInfoList entries = runFolderDir.entryInfoList();

  unsigned int folderCounter = 0;

  for (const QFileInfo& finfo : entries)
  {
    if (finfo.fileName().toStdString().find("Iteration_") != 0)
    {
      continue;
    }

    folderCounter++;
  }

  const QString absolutePathMCMC = runFolderStr + "/Iteration_" + QString::number(folderCounter) + "/" + scenario_.stateFileNameMCMC();
  const QString stateFilename{absolutePathMCMC};
  const QFileInfo checkFile(stateFilename);

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
  out << writeLoadState(scenario_.runLocation() + "/" + scenario_.iterationDirName() + "/" + scenario_.stateFileNameMCMC());
  out << "generateCalibratedCase \"" + filename + "\" 1\n";

  Logger::log() << "- \""  << filename << "\" will be in \"" << scenario_.runLocation() << "\"" << Logger::endl();
}

} // namespace ua

} // namespace casaWizard
