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
  explicit RunOptimalCaseScript(const UAScenario& scenario);

  QString scriptFilename() const override;
  QString workingDirectory() const override;
  QString relativeDirectory() const override;
  QString absoluteDirectory() const;

private:
  void writeScriptContents(QFile& file) const override;

  const UAScenario& scenario_;
  QString projectFile_;
};

} // namespace sac

} // namespace casaWizard
