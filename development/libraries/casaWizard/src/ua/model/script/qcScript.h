// Script to run the sensitivity analysis
#pragma once

#include "uaScript.h"

class QFile;

namespace casaWizard
{

namespace ua
{

class UAScenario;

class QCScript : public UAScript
{
public:
  explicit QCScript(const UAScenario& scenario);

private:
  void writeScriptContents(QFile& file) const override;
};

} // namespace ua

} // namespace casaWizard
