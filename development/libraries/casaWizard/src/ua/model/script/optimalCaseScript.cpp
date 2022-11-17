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

OptimalCaseScript::OptimalCaseScript(const UAScenario& scenario, QString optimalProjectName) :
    CasaScript(scenario.workingDirectory()),
    m_scenario{scenario},
    m_optimalProjectName(optimalProjectName)
{
   m_optimalCaseDirectory = m_scenario.workingDirectory() +
                           "/" + m_scenario.runLocation() +
                           "/Calibrated_" +
                           m_optimalProjectName.left(m_optimalProjectName.indexOf((".")));
}

const CasaScenario& OptimalCaseScript::scenario() const
{
   return m_scenario;
}

QString OptimalCaseScript::scriptFilename() const
{
   return QString("optimalCaseScript.casa");
}

QString OptimalCaseScript::workingDirectory() const
{
   return QString(m_scenario.workingDirectory());
}

QString OptimalCaseScript::optimalCaseDirectory() const
{
   return m_optimalCaseDirectory;
}

bool OptimalCaseScript::generateCommands()
{
   addCommand("rm -rf " + m_optimalCaseDirectory);
   return CasaScript::generateCommands();
}

bool OptimalCaseScript::validateScenario() const
{
   const QString absolutePathMCMC = m_scenario.runCaseSetFileManager().iterationDirPath() + "/" + m_scenario.stateFileNameMCMC();
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
   QTextStream out(&file);
   out << writeBaseProject(m_scenario.project3dPath());
   out << writeLoadState(m_scenario.runLocation() + "/" + m_scenario.iterationDirName() + "/" + m_scenario.stateFileNameMCMC());
   out << "generateCalibratedCase \"" + m_optimalProjectName + "\" 1\n";

   Logger::log() << "- \""  << m_optimalProjectName << "\" will be in \"" << m_scenario.runLocation() << "\"" << Logger::endl();
}

} // namespace ua

} // namespace casaWizard
