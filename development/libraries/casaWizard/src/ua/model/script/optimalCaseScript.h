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
  explicit OptimalCaseScript(const UAScenario& scenario);
  const CasaScenario& scenario() const override;
  QString scriptFilename() const;
  QString workingDirectory() const override;

  QString optimalCaseDirectory() const;

private:
  bool generateCommands() override;
  void writeScriptContents(QFile& file) const override;
  bool validateScenario() const override;

  const UAScenario& scenario_;
  QString optimalCaseDirectory_;
};

} // namespace ua

} // namespace casaWizard
