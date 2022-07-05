#include "runOptimalCaseScript.h"

#include "model/uaScenario.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace ua
{

RunOptimalCaseScript::RunOptimalCaseScript(const UAScenario& scenario) :
  DoEScript(scenario),
  scenario_{scenario},
  projectFile_{"Project.project3d"}
{
}

QString RunOptimalCaseScript::scriptFilename() const
{
  return QString("runOptimalCaseScript.casa");
}

QString RunOptimalCaseScript::workingDirectory() const
{
  return QString(scenario_.workingDirectory());
}

QString RunOptimalCaseScript::relativeDirectory() const
{
  return scenario_.runLocation() + "/Calibrated_Project";
}

QString RunOptimalCaseScript::absoluteDirectory() const
{
  return scenario_.workingDirectory() + "/" + relativeDirectory() + "/";
}

void RunOptimalCaseScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);
  out << writeApp(scenario_.numberCPUs(), scenario_.applicationName());
  out << writeBaseProject(absoluteDirectory() + projectFile_);
  out << QString("doe \"BaseCase\"\n");
  out << writeLocation(scenario_.runLocation());
  out << writeRun(scenario_.clusterName());

  writeTargets(out, true);
  out << "exportDataTxt \"RunCasesObservables\" \"" + scenario_.runCasesObservablesTextFileName() + "\"\n";
}

} // namespace sac

} // namespace casaWizard
