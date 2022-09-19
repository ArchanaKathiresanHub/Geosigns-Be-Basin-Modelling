// Class for running the optimal case of the MCMC
#pragma once

#include "model/script/doeScript.h"

#include <QString>

class QFile;

namespace casaWizard
{

namespace ua
{

class UAScenario;

class RunOptimalCaseScript : public DoEScript
{
public:
  explicit RunOptimalCaseScript(UAScenario& scenario);

  QString scriptFilename() const final;
  QString workingDirectory() const final;
  QString relativeDirectory() const final;
  QString absoluteDirectory() const;

private:
  void writeScriptContents(QFile& file) const final;

  const UAScenario& scenario_;
  QString projectFile_;
};

} // namespace sac

} // namespace casaWizard
