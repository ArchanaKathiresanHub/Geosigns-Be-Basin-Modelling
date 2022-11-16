// Script to export the calibrated case from the MCMC run
#pragma once

#include "model/script/casaScript.h"

class QFile;
class QString;

namespace casaWizard
{

namespace ua
{

class UAScenario;

class OptimalCaseScript : public CasaScript
{
public:
  explicit OptimalCaseScript(const UAScenario& scenario, QString optimalProjectName = "Project.project3d");
  const CasaScenario& scenario() const final;
  QString scriptFilename() const final;
  QString workingDirectory() const final;

  QString optimalCaseDirectory() const;

private:
  bool generateCommands() override;
  void writeScriptContents(QFile& file) const final;
  bool validateScenario() const final;

  const UAScenario& m_scenario;
  QString m_optimalCaseDirectory;
  QString m_optimalProjectName;
};

} // namespace ua

} // namespace casaWizard
